+++
title = "Let's challenge common arguments in favor of microservices"
date = 2024-10-28T09:42:14+01:00
type = "post"
draft = true
+++

https://www.linkedin.com/pulse/why-you-should-use-microservice-architecture-lee-atchison-ouxgc/

**Disclaimer:** The following is an opinion piece about why I think the choice to implement an application in the form of distributed microservices is sometimes made for the wrong reasons. It is not extensively researched and might very well contain factual errors. It's main purpose is to allow me to order my own thoughts on the matter.

Microservices are a well-established design pattern for large software projects, where an application is split into independent components, usually communicating via a network-based interface. Oftentimes, HTTP is used as the protocol to exchange data between microservices, which also allows easily distributing services over several machines. [This 2014 article by Martin Fowler](https://martinfowler.com/articles/microservices.html) explains the concept in some detail.

There are many purported advantages of a software design like this, ranging from purely technical to organizational ones. However, in my opinion many of those are not very convincing when it comes to motivate moving to a microservice architecture. Let's go through a few of the popular arguments and try to challenge them.

## Independent Scaling

### The argument:

If only a certain subcomponent of an application starts experiencing higher load, factoring this component out into a microservice and deploying it on a different machine allows scaling it up independently of the rest of the application, either vertically by increasing the amount of resources that the server running the microservice has available, or horizontally by deploying multiple instances of the same service on multiple machines.

### My take:

I think this is indeed a strong argument for using microservices, especially nowadays where many applications, especially in the world of web programming, are deployed on virtual machines and scaling up a single node requires just a change in some config file. If components of an application communicate via a serial protocol, routing this traffic via the network between machines becomes trivial, and the hardware running some component of a larger system can thus be scaled up completely independent of the other components.

On the other hand of course, in a monolithic design running on a single node, that machine can also be scaled up on demand, and a single component of that system which requires more resources will just use a larger amount of the host's shared resources. However, it is conceivable that splitting the resource load of an application over multiple machines instead may allow for a cheaper deployment, as multiple weak systems can be cheaper to provision than a single beefy one.

So to reiterate, I'm pretty sure that concerns for scalability and deploying an application on a distributed hardware architecture may absolutely be a convincing cause for using microservices. Obviously, it still needs to be considered if and by how much such a deployment actually benefits an application on a per-case basis.

## Independent Deployability

### The argument:

Large projects often employ multiple teams to work on different parts of an application. If every team owns their separate microservice, with a clearly defined interface to the other services, it becomes possible to independently (re)deploy parts of an application, e.g. after a bug fix. As long as the interface to the outside world does not change, other services can just keep on using the updated microservice without any downtime.

In the [article](https://martinfowler.com/articles/microservices.html) I linked at the beginning of this post, Martin Fowler states:

```
Change cycles are tied together - a change made to a small part of the application,
requires the entire monolith to be rebuilt and deployed.
```

### My take:

I think this argument misses the fact that monolithic applications also can be designed in a way that allows hot-reloading some of their components at runtime, via the use of shared libraries (e.g. .dll files on Windows or .jar files for the JVM).

In such a design, one can have a central application, which periodically or on some trigger condition reloads the shared library containing the code of an external component. To deploy a new version of the component, one then only needs to exchange the shared library's binary, without having to restart the whole application.

In desktop GUI applications (e.g. Computer Games), where the idea of microservices is not really a thing, this pattern is sometimes used to facilitate quick development iterations. See e.g. [this video of Casey Muratori's "Handmade Hero" series](https://www.youtube.com/watch?v=WMSBRk5WG58), where this technique is demonstrated in detail. In my opinion, it can just as well be employed to disentangle deployments of multiple teams for a web-service, although I have to confess I have not seen it used in this way in the real world before.

Of course, with a design like this, all components are hosted by the same process and share the same address space. As an alternative solution that is somewhat closer to "actual" microservices, one could also deploy each component as a separate process on the same node and designate a shared memory region for communication. One further step would be to get rid of the shared memory and exchange data between the processes via a socket connection, at which point we basically arrived at what a microservice architecture is anyway :). This goes to show that the concepts of distributed microservices are not far removed from these suggested alternatives, only the implementation differs.

All in all, I don't think the argument for independent deployability of microservices holds much water; if this is the only reason you decided for a microservice architecture, it would probably be a good idea to reconsider in favor of a simpler, more local solution.

## Separation of Concerns

### The argument:

With stable interfaces between microservices, ideally members of a team can stop concerning themselves with the internal implementation details of another service altogether and solely focus on architecting their own service, reducing the mental load required to reason about the system.

### My take:


## Technological Ownership



