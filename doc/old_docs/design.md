# Introduction and Goals

SoftwareContainer (aka Software Container) is used to run applications in a contained environment where access to the main system can be customized using a set of gateways. It is using LXC to setup and control the containers, which in turn is based on kernel namespaces.

# Constraints
There is a tradeoff to be done on how much functionality dealing with e.g. the configuration of gateways etc that should be put in softwarecontainer and how much that should be left for the launcher to handle. The current design is based on the idea that softwarecontainer should remain a layer above LXC that adds the concept for gateways for allow access to the host system.

# Context
SoftwareContainer, mainly softwarecontainer-agent, is meant to be used by other components rather than the user interacting with it manually. In a typical case a "launcher", a component responsible for handling the lifecycle of applications, will use softwarecontainer to run application in containers. This launcher is then responsible for providing the configuration for the different gateways that control what the application can access.

# Solution Ideas and Strategy
## Introduction

SoftwareContainer consist of a few different parts
- softwarecontainer-agent
- softwarecontainer-agent-lib
- libsoftwarecontainer
- softwarecontainer

SoftwareContainer is written in C++, it and uses glibmm for e.g. mainloop and signal handling. It currently use dbus-c++ for DBus access, but this should moved to glibmm as well in the future.

### softwarecontainer-agent
SoftwareContainer-agent is a DBus service which you can communicate with to start and control apps running in containers. One benefit of having this functionality exposed as a DBus service is that it can run as root and then allow the users of the service to run as non-root user and still be able to create and configure containers.

An instance of softwarecontainer-agent handles multiple containers.

### libsoftwarecontainer-agent
This is a library for interfacing with softwarecontainer-agent over DBus, making it convenient to use from C++.

### libsoftwarecontainer
A library which allows you to create and manage containers, used by both softwarecontainer-agent and the stand-alone softwarecontainer.

### softwarecontainer
A stand-alone binary for launching an application in a container. Should be largely obsoleted by softwarecontainer-agent, but is current kept for backwards compatibility.

An instance of softwarecontainer handles only one container.

## Gateways
In order to let applications run inside of containers to access selected parts of the host systems the concepts of gateways is introduced. Gateways are dynamically configured "holes" to the host system. Examples of gateways are:
- Device Node Gateway which allow access to arbitrary device nodes of the host system
- File System Gateway which allow access to selected parts of the host file system
- Network Gateway which enables network access
- DBus Gateway which allows DBus access to all or parts of the available DBus services and interfaces.

# Building Block View

![component-overview](images/component_overview.png  "Component Overview")

# Runtime View

# Deployment View

# Concepts

# Design Decisions

# Quality Scenarios

# Risks

# Glossary
- SC - Software Container (official name of softwarecontainer)

# Document TODO
List of tasks related to this documentation.
