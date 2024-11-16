+++
title = "Challenge of common arguments in favor of microservices"
date = 2024-11-16T09:42:14+01:00
type = "post"
draft = false
+++

**Disclaimer:** The following is an opinion piece about why I think the choice to implement an application in the form of distributed microservices is sometimes made for the wrong reasons. It is not extensively researched and might very well contain factual errors. It's main purpose is to allow me to order my own thoughts on the matter.

Microservices are a well-established design pattern for large software projects, where an application is split into independent components, usually communicating via a network-based interface. Oftentimes, HTTP is used as the protocol to exchange data between microservices, which also allows easily distributing services over several machines. [This 2014 article by Martin Fowler](https://martinfowler.com/articles/microservices.html) explains the concept in some detail.

There are many purported advantages of a software design like this, ranging from purely technical to organizational ones. However, in my opinion many of those are not very convincing when it comes to motivate moving to a microservice architecture. 

## The problem with microservices

But first, why would the choice to use microservices require motivation in the first place? Do they bring any inherent drawbacks that might negatively affect an application?

The biggest and most glaring drawback is the significant overhead when exchanging data between distinct processes, especially if they run on different machines and need to communicate via the network. The [Latency Numbers Everyone Should Know](https://static.googleusercontent.com/media/sre.google/en//static/pdf/rule-of-thumb-latency-numbers-letter.pdf) table has been going around for many years now. In it, we can see that a round trip between nodes in the same datacenter incurs a latency of ~500us. In addition, to send data over the network it needs to be serialized on the sender's and parsed on the receiver's side, which adds additional overhead. The serialization overhead needs to be paid even if microservices run on the same node.

In comparison, in a monolithic application all components write to and read from the same memory device. The table denotes the latency for referencing main memory as ~100ns, which is lower than the network roundtrip by a factor of 5000. If all components of the application use the same memory layout, no serialization or parsing is required.

A secondary argument against the use of microservices is the added complexity of setting up multiple programs, possibly on multiple nodes, each with their own message passing stack to communicate with the rest of the system. This also makes debugging of errors harder, as the control and data flow needs to be followed across process boundaries.

In my opinion, these are very compelling arguments for defaulting to a monolithic application whenever it is feasible, and only start splitting off components into microservices as soon as strong arguments to do so arise.

## Thoughts on common microservice arguments

With the motivation for why microservices should be critically examined out of the way, let's go through a few of the popular arguments in favor of them and try to challenge those.

### Independent Scaling

#### The argument:

If only a certain subcomponent of an application starts experiencing higher load, factoring this component out into a microservice and deploying it on a different machine allows scaling it up independently of the rest of the application, either vertically by increasing the amount of resources that the server running the microservice has available, or horizontally by deploying multiple instances of the same service on multiple machines.

#### My take:

I think this is indeed a strong argument for using microservices, especially nowadays where many applications, especially in the world of web programming, are deployed on virtual machines and scaling up a single node requires just a change in some config file. If components of an application communicate via a serial protocol, routing this traffic via the network between machines becomes trivial, and the hardware running some component of a larger system can thus be scaled up completely independent of the other components.

On the other hand of course, in a monolithic design running on a single node, that machine can also be scaled up on demand, and a single component of that system which requires more resources will just use a larger amount of the host's shared resources. However, it is conceivable that splitting the resource load of an application over multiple machines instead may allow for a cheaper deployment, as multiple weak systems can be cheaper to provision than a single beefy one.

So to reiterate, I'm pretty sure that concerns for scalability and deploying an application on a distributed hardware architecture may absolutely be a convincing cause for using microservices. Obviously, it still needs to be considered if and by how much such a deployment actually benefits an application on a per-case basis.

### Independent Deployability

#### The argument:

Large projects often employ multiple teams to work on different parts of an application. If every team owns their separate microservice, with a clearly defined interface to the other services, it becomes possible to independently (re)deploy parts of an application, e.g. after a bug fix. As long as the interface to the outside world does not change, other services can just keep on using the updated microservice without any downtime.

In the [article](https://martinfowler.com/articles/microservices.html) I linked at the beginning of this post, Martin Fowler states:

```
Change cycles are tied together - a change made to a small part of the application,
requires the entire monolith to be rebuilt and deployed.
```

#### My take:

I think this argument misses the fact that monolithic applications also can be designed in a way that allows hot-reloading some of their components at runtime, via the use of shared libraries (e.g. .dll files on Windows or .jar files for the JVM).

In such a design, one can have a central application, which periodically or on some trigger condition reloads the shared library containing the code of an external component. To deploy a new version of the component, one then only needs to exchange the shared library's binary, without having to restart the whole application.

In desktop GUI applications (e.g. Computer Games), where the idea of microservices is not really a thing, this pattern is sometimes used to facilitate quick development iterations. See e.g. [this video of Casey Muratori's "Handmade Hero" series](https://www.youtube.com/watch?v=WMSBRk5WG58), where this technique is demonstrated in detail. In my opinion, it can just as well be employed to disentangle deployments of multiple teams for a web-service, although I have to confess I have not seen it used in this way in the real world before.

Of course, with a design like this, all components are hosted by the same process and share the same address space. As an alternative solution that is somewhat closer to "actual" microservices, one could also deploy each component as a separate process on the same node and designate a shared memory region for communication. One further step would be to get rid of the shared memory and exchange data between the processes via a socket connection, at which point we basically arrived at what a microservice architecture is anyway :). This goes to show that the concepts of distributed microservices are not far removed from these suggested alternatives, only the implementation differs.

All in all, I don't think the argument for independent deployability of microservices holds much water; if this is the only reason you decided for a microservice architecture, it would probably be a good idea to reconsider in favor of a simpler, more local solution.

### Separation of Concerns and Technical Ownership

#### The argument:

With stable interfaces between microservices, ideally members of a team can stop concerning themselves with the internal implementation details of another service altogether and solely focus on developing their own service. By reducing all other components to just their interfaces, the mental load required to reason about the full system is reduced.

In addition, the choice of technology becomes entirely independent of the surrounding systems. As long as a service satisfies the agreed upon interface contract, it is free to use any programming language, libraries or frameworks the team desires.

#### My take:

I'd like to hark back here to my thoughts about the "Independent Deployability" argument above, as I think they apply here as well: The advantages stated in this argument are not exclusive to microservices. If an application consists of multiple libraries, each with a clearly defined interface, it is also not required for developers of one library to have in-depth knowledge about the internal details of another.

Also, it is entirely possible for a shared library to be implemented on another technology stack than the client program, as long as a stable binary interface is provided. For example, many programming languages' standard libraries provide facilities to create wrappers in C, which in turn describes the defacto standard interface to call into shared libraries. So e.g. a library written in Go can expose its public functions via a C interface, and a client written in Python can then call into this interface. The mechanism to allow a program written in one programming language to call into code written in another one is commonly known as a *Foreign Function Interface(FFI)*.

I do concede that working with multiple FFIs in the case of many components in different languages doesn't sound fun and would probably increase the system complexity compared to serializing all procedure calls into messages the same way and sending them via a socket connection (which would be the alternative in microservice-world). Then again, maybe using so many different technologies that this becomes cumbersome is a smell of a bad design in the first place.

## Closing thoughts

I hope this post doesn't come off as too rambly. Like I mentioned in the beginning, I wanted to jot down my thoughts about microservices and when (not) to use them in a more-or-less structured manner in order to help my own understanding.

As a final verdict, I'd say I remain skeptical of the microservice pattern. While it does lend itself well to the practice of horizontal scaling for high-throughput applications (and as I mentioned earlier, I think scaling is a strong argument in favor of using microservices), I have seen in some previous jobs that this topic suffers from some amount of cargo culting and there are probably many applications in the world implemented as several microservices, which would benefit from a more monolithic design.

One thing to keep in mind is that the decision between microservices and a monolith is not binary. It is perfectly possible to split off a single component of an application into a microservice, for example to allow independent scaling of that component, while keeping the rest of the monolith intact. With this understanding, microservices become just another tool for a Software Architect, one which I think should be used with caution.