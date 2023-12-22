# V8AndGUI

This is a small demo showing how to embed and bind V8 in a graphics or GUI centered application.

The main points to be shown:

* Interface description using a set of structs (definitions.hpp), which are used in DefaultModule.cpp and the wrapper classes to define the functions/objects/classes to be exported to the JavaScript space. This coding pattern is developed during the recent restructuring of a bigger project [Three.V8](https://github.com/fynv/three.v8).
  
* Working with async callbacks. This kind of applications normally have their own event loops. The demo shows 2 ways of making the callbacks: using the system message queue (for GUI elements) and using a separate set of message queues (ConcurrentQueue.h, AsyncCallbacks.cpp, for network requests).

I find [Dear ImGui](https://github.com/ocornut/imgui) to be helpful for providing a minimal background for showing the patterns. 

The "Launcher" window is coded directly into the main program, which is used for loading code and showing debug log. The scripts and use the following APIs exported by the main program:

* Global function "print()" for printing log.
* Global object "http" for making synced/asynced http requests.
* Global object "scriptWindow" for adding UI elements to the 2nd window.
* UI element classes: "Text", "SameLine", "InputText", and "Button".



