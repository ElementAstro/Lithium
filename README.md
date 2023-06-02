# OpenAPT
Open Astrophotography Terminal

## Why we build this software ?

In the past, astronomical photography was a tedious and frustrating process. Enthusiasts had to battle with unpredictable weather conditions and constantly monitor their cameras to ensure the perfect shot. Carrying around bulky equipment only added to the hassle, making it a far from enjoyable experience.

While commercial products have emerged to make things easier, they come with major downsides. Many of these solutions are closed source and lack compatibility with devices that aren't their own, leaving astronomers feeling frustrated and limited in their choices.

We understand this frustration all too well, which is why we've set out to create an open source alternative - the Astronomical Pi. With a passion for the cosmos and a deep commitment to open source principles, we're determined to make astronomical photography accessible to everyone.

Our team is passionate about creating an affordable and easy-to-use solution that not only supports all mainstream operating systems and astronomical devices, but also offers a highly optimized asynchronous architecture for stable and efficient performance. Our software even includes modified versions of popular tools like PHD2, asap, and astrometry, making it easier than ever to capture breathtaking images.

Despite facing stiff competition from closed source alternatives, we're not willing to give up. We believe that the world belongs to open source, and we're committed to making our vision a reality.

## Features

OpenAPT is an astronomy software that caters to astronomers and astrophotographers of all levels. It boasts stable and efficient performance on mainstream operating systems, and seamless integration with most astronomical devices. With a modified version of PHD2 and asap and astrometry as platesolving engines, capturing stunning images has never been easier. Its multiple interface access, including web, desktop, and terminal interfaces, makes it user-friendly from anywhere. 

But what sets OpenAPT apart is its commitment to accessibility and open source. Based on GPL3, it's completely free and customizable to suit unique needs. OpenAPT's intuitive and adaptable system allows for easy management and control of multiple astronomical devices, opening up the cosmos to enthusiasts and professionals alike.

## How to build

### Install dependencies

```
sudo apt-add-repository ppa:mutlaqja/ppa -y // Add INDI source
sudo apt update && sudo apt upgrade -y 
sudo apt install libspdlog-dev libboost-dev libgsl-dev libcfitsio-dev libz-dev python3-dev libssl-dev libboost-system-dev && sudo apt-get install libindi1 indi-bin libindi-dev
```

or just run

```
sudo sh scripts/build_ci.sh
```

### Build the code

```
mkdir build && cd build
cmake ..
make -j4

./openapt
```

Everything is very simple.

```
Learning requires not mere imagination,
Nor can it be attained through mediocre dedication.
It is through diligent accumulation,
That we shall grow in our education.

Our efforts may falter and fail,
But we must not surrender and bail.
We shall not halt our stride for fear of stumbling,
For setbacks are the price of pursuing enlightenment.

On this quest for truth, we shall encounter obstacles and doubts,
Yet we shall keep our resolve to seek understanding throughout.
Let us nurture a heart that yearns for wisdom and grace,
And never lose sight of this noble race.
```