# Project Data for Deepening – UDP Video Streaming in NS-3

This project investigates how network latency affects the quality of UDP-based video streaming using the NS-3 simulator (version 3.41).  
A custom simulation script (`udp_video_sim.cc`) was developed to model a point-to-point connection between a client and a server, where latency values were varied between 20–200 ms.

Metrics such as packet loss, delay, and jitter were collected using FlowMonitor to analyze how network performance impacts Quality of Service (QoS) and Quality of Experience (QoE).  
The results demonstrate that increasing network delay significantly affects video streaming performance, especially when using UDP.

**Author:** Ayah Salem  
**Institution:** University West, Department of Computer Engineering  
**Year:** 2025

