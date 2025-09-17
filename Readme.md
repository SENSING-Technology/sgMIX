### sgMIX Demo
The `demo` program is provided as a sample application for testing the SDK.  
It demonstrates how to read the intrinsic parameters of a monocular camera.  
For more SDK features, please refer to the header files in the `include` directory.

#### SDK Directory
- `include/` : Header files for development  
- `lib/` : Static and shared libraries  
- `samples/` : Example code demonstrating SDK usage  
- `app/` : GUI application integrated with the SDK

#### Compile and run the program
```
mkdir build && cd build 
cmake ..
make
sudo ./demo
```