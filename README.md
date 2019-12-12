# Home Security Suite (home-security)

## About
This project is a home security suite based on jetson-inference. It is tailored to run only on a [NVIDIA Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)

Refer to [get-started-jetson-nano-devkit](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit) for complete setup of the developer kit.

Refer to [jetson-projects](https://developer.nvidia.com/embedded/community/jetson-projects) for examples on other jetson nano projects.

Jetson inference [on github](https://github.com/dusty-nv/jetson-inference) is the inference and realtime DNN (Deep Neural Network) vision library for NVIDIA Jetson Nano/TX1/TX2/Xavier. The READ.me of this git repo contains all relevant information to build and deploy the jetson-inference environment.

Once the Jetson Nano Developers Kit is installed, up and running take the following steps.
On the Jetson Nano, open a new Terminal.

* Install pre-requisite packages
```
sudo apt-get install libcurl4-nss-dev
sudo apt-get install git cmake libpython3-dev python3-numpy
sudo apt-get update
```

* Checkout git repositories
```
mkdir -p ~/github
cd ~/github
git clone --recursive https://github.com/dusty-nv/jetson-inference
git clone --recursive https://github.com/harmcoenen/home-security
```

* Build and install jetson-inference
```
cd ~/github/jetson-inference/
mkdir build
cd build/
cmake ../
make
sudo make install
sudo ldconfig
```

* Download more pre-trained models if you like
```
cd ../tools/
./download-models.sh 
```

* Run some example applications
```
cd ../build/aarch64/bin/
./imagenet-console bear_0.jpg output_0.jpg
./imagenet-console --network=googlenet images/orange_0.jpg output_0.jpg
./imagenet-camera
./imagenet-camera googlenet
./imagenet-camera --camera=/dev/video0 --width=640 --height=480
./detectnet-camera
./detectnet-console dogs.jpg output.jpg coco-dog
./detectnet-console peds.jpg output.jpg multiped
./my-recognition test-image.jpg
```

* Build and install home-security
```
cd ~/github/home-security/
mkdir build
cd build/
cmake ../
make
sudo make install
sudo ldconfig
```

* Run home-security
```
home-security
```
