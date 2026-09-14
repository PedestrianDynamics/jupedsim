################################################################################
# floorfield (Rust crate via cxx)
################################################################################
corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml)
corrosion_add_cxxbridge(floorfield_cxx
    CRATE floorfield
    FILES lib.rs
)