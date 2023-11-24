use crate::input::HasPackets;
use libafl::{
    inputs::{bytes::BytesInput, Input},
    mutators::{MutationResult, Mutator, MutatorsTuple},
    state::{HasMaxSize, HasRand},
    mutators::MutationId,
    Error,
};
use std::marker::PhantomData;
use libafl_bolts::{rands::Rand, HasLen, Named};


pub trait HasHavocMutation<MT, S>
    where
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    fn mutate_havoc(&mut self, state: &mut S, mutations: &mut MT, mutation: MutationId, stage_idx: i32) -> Result<MutationResult, Error>;
}

impl<MT, S> HasHavocMutation<MT, S> for BytesInput
    where
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    fn mutate_havoc(&mut self, state: &mut S, mutations: &mut MT, mutation: MutationId, stage_idx: i32) -> Result<MutationResult, Error> {
        mutations.get_and_mutate(mutation, state, self, stage_idx)
    }
}

pub struct PacketHavocMutator<I, MT, S, P>
    where
        P: HasHavocMutation<MT, S>,
        I: Input + HasLen + HasPackets<P>,
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    mutations: MT,
    phantom: PhantomData<(I, S, P)>,
}

impl<I, MT, S, P> PacketHavocMutator<I, MT, S, P>
    where
        P: HasHavocMutation<MT, S>,
        I: Input + HasLen + HasPackets<P>,
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    pub fn new(mutations: MT) -> Self {
        Self {
            mutations,
            phantom: PhantomData,
        }
    }

    fn iterations(&self, state: &mut S) -> u64 {
        state.rand_mut().below(16) as u64
    }

    fn schedule(&self, state: &mut S) -> MutationId {
        state.rand_mut().below(self.mutations.len() as u64).into()
    }
}

impl<I, MT, S, P> Mutator<I, S> for PacketHavocMutator<I, MT, S, P>
    where
        P: HasHavocMutation<MT, S>,
        I: Input + HasLen + HasPackets<P>,
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    fn mutate(&mut self, state: &mut S, input: &mut I, stage_idx: i32) -> Result<MutationResult, Error> {
        if input.len() == 0 {
            return Ok(MutationResult::Skipped);
        }

        let mut result = MutationResult::Skipped;
        let iters = self.iterations(state);
        let packet = state.rand_mut().below(input.len() as u64) as usize;

        for _ in 0..iters {
            let mutation = self.schedule(state);

            let outcome = input.packets_mut()[packet].mutate_havoc(state, &mut self.mutations, mutation, stage_idx)?;

            if outcome == MutationResult::Mutated {
                result = MutationResult::Mutated;
            }
        }

        Ok(result)
    }
}

impl<I, MT, S, P> Named for PacketHavocMutator<I, MT, S, P>
    where
        P: HasHavocMutation<MT, S>,
        I: Input + HasLen + HasPackets<P>,
        MT: MutatorsTuple<BytesInput, S>,
        S: HasRand + HasMaxSize,
{
    fn name(&self) -> &str {
        "PacketHavocMutator"
    }
}
