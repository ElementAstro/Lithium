# OpenAPT
Open Astrophotography Terminal

## Why we build this software ?

In the past, astronomical photography was a tedious and frustrating process. Enthusiasts had to battle with unpredictable weather conditions and constantly monitor their cameras to ensure the perfect shot. Carrying around bulky equipment only added to the hassle, making it a far from enjoyable experience.

While commercial products have emerged to make things easier, they come with major downsides. Many of these solutions are closed source and lack compatibility with devices that aren't their own, leaving astronomers feeling frustrated and limited in their choices.

We understand this frustration all too well, which is why we've set out to create an open source alternative - the Astronomical Pi. With a passion for the cosmos and a deep commitment to open source principles, we're determined to make astronomical photography accessible to everyone.

Our team is passionate about creating an affordable and easy-to-use solution that not only supports all mainstream operating systems and astronomical devices, but also offers a highly optimized asynchronous architecture for stable and efficient performance. Our software even includes modified versions of popular tools like PHD2, asap, and astrometry, making it easier than ever to capture breathtaking images.

Despite facing stiff competition from closed source alternatives, we're not willing to give up. We believe that the world belongs to open source, and we're committed to making our vision a reality.

## Features

Our astronomy software is designed to meet the needs of astronomers and astrophotographers of all levels. We have extensively optimized our platform for stable and efficient performance on all mainstream operating systems. Our device driver, based on INDI and ASCOM technologies, supports most astronomical devices - ensuring seamless integration with your existing equipment.

Our software also includes a modified version of PHD2, built in as the guide tool. With asap and astrometry as the platesolving engine, you can capture stunning images with ease. And with multiple interface access, including web, desktop, and even terminal interfaces, it's easy to use no matter where you are.

We're also proud to be fully open source, based on GPL3. This means that you can customize our software to suit your own unique needs, and it's completely free to use. We believe in making astronomy accessible to everyone, and we're committed to opening up the cosmos to enthusiasts and professionals alike.

Our OpenAPT system follows a unique approach that enables seamless communication between various astronomical devices and client processes. The OpenAPT clients use INET websockets to connect with the OpenAPT server, which acts as a central hub for exchanging data and instructions between the clients and drivers.

Each OpenAPT client is associated with a specific driver that is responsible for managing the communication with the associated astronomical device. For example, OpenAPT Client 1 communicates with OpenAPT Driver A, which in turn communicates with device X. Similarly, OpenAPT Client 2 communicates with OpenAPT Driver B that controls device Y, and so on. The OpenAPT drivers are designed to handle all necessary low-level device-specific communication protocols, enabling the clients to interact with the devices at a higher level of abstraction.

The OpenAPT server operates as the mediator between the clients and drivers, ensuring that incoming data from the clients is routed to the appropriate driver, and vice versa. The server also handles synchronization between the clients and drivers, ensuring that data is shared accurately and efficiently.

Our system is highly modular, making it easy to add support for new devices or extend the functionality of existing drivers. All hardware devices are controlled through a common API interface implemented by the respective drivers, ensuring consistency and ease of use across different devices.

With OpenAPT, astronomers can now easily manage and control multiple astronomical devices using a unified interface. Our system is designed to be intuitive, efficient, and highly adaptable to the changing needs of astronomers and astrophotographers.

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