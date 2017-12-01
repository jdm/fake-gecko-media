extern crate cmake;

fn main() {
    let dst = cmake::build("gecko-media");
    println!("cargo:rustc-link-search=native={}", dst.display());
    println!("cargo:rustc-link-lib=static=gecko_media_cmake");

    println!("cargo:rerun-if-changed=CMakeLists.txt");
    println!("cargo:rerun-if-changed=gecko-media/GeckoMedia.cpp");
    println!("cargo:rerun-if-changed=gecko-media/GeckoMedia.h");
    println!("cargo:rerun-if-changed=gecko-media/MediaFormatReader.cpp");
    println!("cargo:rerun-if-changed=gecko-media/MediaFormatReader.h");
}
