use libafl::inputs::{BytesInput, Input};
use libafl_bolts::HasLen;
use serde::{Deserialize, Serialize};

pub trait HasPackets<I> {
    /// Get the inputs packets
    fn packets(&self) -> &[I];

    /// Get the inputs packets
    fn packets_mut(&mut self) -> &mut Vec<I>;
}

#[derive(Serialize, Deserialize, Clone, Debug, Default, PartialEq, Eq, Hash)]
pub struct PacketInput {
    packets: Vec<BytesInput>,
}

impl PacketInput {
    #[must_use]
    pub fn new(packets: Vec<BytesInput>) -> Self {
        Self { packets }
    }

    #[must_use]
    pub fn packets(&self) -> &[BytesInput] {
        &self.packets
    }

    #[must_use]
    pub fn packets_mut(&mut self) -> &mut Vec<BytesInput> {
        &mut self.packets
    }
}

impl Input for PacketInput {
    fn generate_name(&self, idx: usize) -> String {
        format!("id_{idx}")
    }
}

impl HasPackets<BytesInput> for PacketInput {
    fn packets(&self) -> &[BytesInput] {
        &self.packets
    }

    fn packets_mut(&mut self) -> &mut Vec<BytesInput> {
        &mut self.packets
    }
}

impl HasLen for PacketInput {
    fn len(&self) -> usize {
        self.packets.len()
    }
}
