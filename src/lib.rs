pub mod bindings {
    extern "C" {
        pub fn GeckoMedia_QueueRustRunnable(); 
    }
}

/*pub struct GeckoMedia;
impl GeckoMedia {
    pub fn shutdown() -> Result<(), ()> {
        Ok(())
    }

    pub fn queue_task(&self) {
        unsafe { bindings::GeckoMedia_QueueRustRunnable() };
    }
}*/
pub fn foo() {
    unsafe { bindings::GeckoMedia_QueueRustRunnable() };
}
