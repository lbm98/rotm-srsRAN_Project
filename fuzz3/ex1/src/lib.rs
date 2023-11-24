use core::time::Duration;
use std::{fs, path::PathBuf};

use libafl::{
    corpus::{Corpus, CachedOnDiskCorpus, OnDiskCorpus},
    events::{setup_restarting_mgr_std, EventConfig, EventRestarter},
    executors::{inprocess::InProcessExecutor, ExitKind, TimeoutExecutor},
    feedback_or, feedback_or_fast,
    feedbacks::{CrashFeedback, MaxMapFeedback, TimeFeedback, TimeoutFeedback},
    fuzzer::{Fuzzer, StdFuzzer},
    inputs::{BytesInput, HasTargetBytes},
    monitors::MultiMonitor,
    mutators::{scheduled::{havoc_mutations, StdScheduledMutator}},
    observers::{HitcountsMapObserver, TimeObserver},
    schedulers::{IndexesLenTimeMinimizerScheduler, QueueScheduler},
    state::{HasCorpus, StdState},
    stages::{StdMutationalStage},
    Error,
};

use libafl_bolts::{
    rands::StdRand,
    tuples::{tuple_list},
    AsSlice,
};

use libafl_targets::{edges_max_num, libfuzzer_test_one_input, std_edges_map_observer};

use clap::{self, Parser};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Opt {
    #[arg(short, long, help = "Set an initial corpus directory", name = "INPUT")]
    input_dir: Vec<PathBuf>,

    #[arg(short, long, help = "Set the output directory", name = "OUTPUT")]
    output_dir: PathBuf,
}

#[no_mangle]
pub extern "C" fn libafl_main() {

    let opt = Opt::parse();

    let broker_port = 1337;

    let mut out_dir = opt.output_dir.clone();
    if fs::create_dir(&out_dir).is_err() {
        log::info!("Out dir at {:?} already exists.", &out_dir);
        assert!(
            out_dir.is_dir(),
            "Out dir at {:?} is not a valid directory!",
            &out_dir
        );
    }
    let mut crashes = out_dir.clone();
    crashes.push("crashes");
    out_dir.push("queue");

    //
    // Component: Monitor
    //

    // MultiMonitor displays cumulative and per-client statistics.
    // It uses LLMP for communication between broker and client(s).

    let monitor = MultiMonitor::new(|s| println!("{s}"));

    //
    // Component: EventManager
    //

    // The event manager handles the various events generated during the fuzzing loop.

    let (state, mut restarting_mgr) =
        match setup_restarting_mgr_std(monitor, broker_port, EventConfig::AlwaysUnique) {
            Ok(res) => res,
            Err(err) => match err {
                Error::ShuttingDown => {
                    return;
                }
                _ => {
                    panic!("Failed to setup the restarter: {err}");
                }
            },
        };

    //
    // Component: Observer
    //

    // Create an observation channel using the coverage map.
    // The libafl_cc compiler uses SanitizerCoverage.

    let edges_observer = HitcountsMapObserver::new(unsafe { std_edges_map_observer("edges") });

    // Create an observation channel to keep track of the execution time

    let time_observer = TimeObserver::new("time");

    //
    // Component: Feedback
    //

    // A feedback processes the information reported by one or more observers,
    // to decide if the execution is interesting.

    // The MaxMapFeedback attempts to maximize the map contents linked to the edges observer.
    // It will track indexes, but not novelties (true, false).

    let max_map_feedback = MaxMapFeedback::tracking(&edges_observer, true, false);

    // An input is interesting iff MaxMapFeedback classifies it as interesting.

    let mut feedback = feedback_or!(
        max_map_feedback,
        // The TimeFeedback never classifies an input as interesting.
        // However, it does keep track of input execution time.
        TimeFeedback::with_observer(&time_observer)
    );

    // The objective feedback decides if an input solves the fuzzing problem.
    // If an input results in a crash or a timeout, we have found a security vulnerability.

    let mut objective = feedback_or_fast!(CrashFeedback::new(), TimeoutFeedback::new());

    //
    // Component: State
    //

    // On the initial run, setup_restarting_mgr_std returns (None, LlmpRestartingEventManager).
    // On successive runs, it restores the state from the prior run,

    let mut state = state.unwrap_or_else(|| {
        StdState::new(
            StdRand::with_seed(42),

            // Corpus that will be evolved.
            // Store in-memory for performance.
            CachedOnDiskCorpus::new(out_dir.clone(), 4096).unwrap(),

            // Corpus in which we store solutions: inputs resulting in crashes or timeouts.
            // Store on-disk for persistence.
            OnDiskCorpus::new(crashes.clone()).unwrap(),
            &mut feedback,
            &mut objective,
        )
            .unwrap()
    });

    println!("We're a client, let's fuzz :)");

    //
    // Component: Mutator
    //

    let mutator = StdScheduledMutator::new(havoc_mutations());

    //
    // Component: Stage
    //

    let mut stages = tuple_list!(StdMutationalStage::new(mutator));

    //
    // Component: Scheduler
    //

    // A scheduler defines a strategy to retrieve inputs from the corpus.
    // The IndexesLenTimeMinimizerScheduler prioritizes quick and small inputs
    // that exercise all coverage seen so far.

    let scheduler = IndexesLenTimeMinimizerScheduler::new(QueueScheduler::new());

    //
    // Component: Fuzzer
    //

    let mut fuzzer = StdFuzzer::new(scheduler, feedback, objective);

    //
    // Component: Harness
    //

    // The harness function calls out the LLVM-style harness.

    let mut harness = |input: &BytesInput| {
        let target = input.target_bytes();
        let buf = target.as_slice();
        libfuzzer_test_one_input(buf);
        ExitKind::Ok
    };

    //
    // Component: Executor
    //

    // Create an in-process executor.
    // The TimeoutExecutor wraps the InProcessExecutor and sets a timeout before each run.

    let mut executor = TimeoutExecutor::new(
        InProcessExecutor::new(
            &mut harness,
            tuple_list!(edges_observer, time_observer),
            &mut fuzzer,
            &mut state,
            &mut restarting_mgr,
        ).unwrap(),
        Duration::new(10, 0),
    );

    // In case the corpus is empty (as on first run), load existing inputs from on-disk corpus.

    if state.must_load_initial_inputs() {
        state
            .load_initial_inputs(&mut fuzzer, &mut executor, &mut restarting_mgr, &opt.input_dir)
            .unwrap_or_else(|_| panic!("Failed to load initial corpus at {:?}", &opt.input_dir));
        println!("We imported {} inputs from disk.", state.corpus().count());
    }

    // Only run for n iterations.
    // Decrease n if target is unstable.
    // Increase n for more performance.

    let iters = 1_000_000;
    fuzzer.fuzz_loop_for(
        &mut stages,
        &mut executor,
        &mut state,
        &mut restarting_mgr,
        iters,
    ).unwrap();

    // We only run for n iterations.
    // Pass the state to make it available in the next, respawned, iteration.

    restarting_mgr.on_restart(&mut state).unwrap();
}