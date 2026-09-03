#[cxx::bridge(namespace = "floorfield")]
mod ffi {
    extern "Rust" {
        fn add(a: i32, b: i32) -> i32;
        fn greeting() -> String;
    }
}

fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn greeting() -> String {
    "Hello from floorfield (Rust)!".to_string()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add_positive() {
        assert_eq!(add(2, 3), 5);
    }

    #[test]
    fn test_add_negative() {
        assert_eq!(add(-1, 1), 0);
    }

    #[test]
    fn test_greeting_contains_rust() {
        assert!(greeting().contains("Rust"));
    }
}
