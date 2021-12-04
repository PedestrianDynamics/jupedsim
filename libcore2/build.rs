extern crate cbindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let out_dir = env::var("BINDINGS_OUT_PATH").unwrap_or(".".to_string());
    let header_path = PathBuf::from(out_dir).join("libcore2.h");

    cbindgen::generate(&crate_dir)
        .unwrap()
        .write_to_file(header_path);
}
