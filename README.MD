# Multi PRX sleep error example

Two plugins to demonstrate an issue where a thread fails to sleep.

The `Core` plugin has an event. It exports a `CoreWait` method that when called will sleep the executing thread untill the event fires. It also listens to the controller and when `Circle` is pressed emits the event.

The `Overlay` plugin displays a overlay when `Cross` is pressed and logs the response from `sceKernelWaitEventFlag`. When `Triangle` is pressed the `Overlay` plugin calls `CoreWait` from its rendering loop.

## HTR

* With both plugins from the release folder enabled for VSH
* Keep hold of `Cross` for the entire test.
* Press `Triangle`
* After a delay press `Circle`.
* Release `Cross`.

What i would expect to happen is the overlay is hidden after pressing `Triangle` because the renering thread is put to sleep.

What actually happens is the Overlay is remains visible and the response from `sceKernelWaitEventFlag` is logged as `-2147352166` (is this just the lowest value available?)

