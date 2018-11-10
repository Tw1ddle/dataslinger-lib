[![License](https://img.shields.io/badge/License-GPL%20v3-blue.svg?style=flat-square)](https://github.com/Tw1ddle/dataslinger/lib/dataslinger/blob/master/LICENSE)

Data slinger is a C++17 library for passing data between processes and over a network. It presents a simple interface, started with a few lines of code:

dataslinger::connection::ConnectionInfo info{};

// These run on their own threads
dataslinger::slinger::DataSlinger slinger;
dataslinger::slinger::Receiver receiver;


## Notes
 * Got an idea or suggestion? Open an issue on GitHub, or send Sam a message on [Twitter](https://twitter.com/Sam_Twidale).