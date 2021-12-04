use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn hello_rust() -> *const c_char {
    "Hello Rust!".as_ptr() as *const c_char
}

#[no_mangle]
pub extern "C" fn hello_rust2() -> *const c_char {
    "Hello Rust!".as_ptr() as *const c_char
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        let result = 2 + 2;
        assert_eq!(result, 4);
    }
}
