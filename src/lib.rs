pub mod bindings {
    extern "C" {
        pub fn GeckoMedia_QueueRustRunnable(); 
    }
}

pub struct GeckoMedia;
impl GeckoMedia {
    pub fn shutdown() -> Result<(), ()> {
        unsafe { bindings::GeckoMedia_QueueRustRunnable() };
        Ok(())
    }
}
