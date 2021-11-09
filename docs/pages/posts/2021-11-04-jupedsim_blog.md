---
title: "JuPedSim Development"
keywords: projects
tags: [blog]
sidebar: jupedsim_sidebar
folder: posts
permalink: 2021-11-04-jupedsim_blog.html
summary: In this retrospective, we reflect on the journey of the development of this project, hoping to document our progress and learnings.
toc: false
last_updated: Nov 9rd, 2021
---

## Problem description

JuPedSim has been actively developed since July 2012 and consists of
several applications: `jpscore` to simulate pedestrians, `jpsvis` to visualize
trajectories and `jpsreport` to analyze trajectories further. The goal of the
JuPedSim toolset is to enable research into Pedestrian dynamics.

Development on the JuPedSim tools slowed down considerably over the last 1 1/2
years. What happened? People involved with developing JuPedSim realized that
extending or changing the existing tools became too tricky and cumbersome.
Ph.D. candidates, who are already more concerned with their ongoing research,
faced difficulties when using JuPedSim as a platform to experiment with and
express their ideas quickly and intuitively.

Based on this insight, the workgroup got together and discussed other issues
they saw with JuPedSim:

- They had been unable to address long-standing feature requests such as
  storing and resuming simulations or providing the simulation core as a
  library so that it could be used in other projects directly.

- Provide better support to reproduce results.

- The software did crash or spuriously remove agents.

## New beginnings

At a certain point, it seemed that some of the JuPedSim tools had reached the
end of their useful life. Hence, the decision was made to design and implement
a simulation tool from scratch. The team started to discuss the concrete steps
of how to rewrite `jpscore` as a more extensible and, in some aspects, more
intuitive software.

By now, you can already guess since this post is not announcing the new release of
JuPedSim2 that these efforts ultimately did not turn out to work as expected.

Well, to some extent, that is true. Several aspects contributed to the fact
that the rewrite did not progress as desired:

- Writing an application from scratch is a slightly different skillset as
  extending a mostly complete application.

- Coordinating multiple part-time developers is extremely difficult.

- Not all development on JuPedSim tools could be stopped at once since Ph.D.
  students relying on JuPedSim had still to be supported. Ongoing projects have
  continued to require simulations and some bugs still had to be fixed.

After almost one year of development, with no working replacement software in
clear sight, we were forced to rethink our strategy.  Not having a platform to
develop new algorithms turned out to be a severe issue. The answer for us was
more closely evaluating and assessing the needs of the workgroup to stay
productive in light of what has been achieved in the rewrite process so far.

The rewrite efforts of JuPedSim were not in vain, though. Apart from the
lessons learned from this process, we have elaborated:

- a target architecture that addresses our needs to offer a CLI simulation tool
  as a library.

- A target architecture which allows easy integration into other languages,
  e.g., Python, which eases the use and the development of JuPedSim for people
  who are not familiar with C++.

- Ideas on how to address reproducibility.

- How to allow for a more flexible design that helps extend the simulation
  easier with new algorithms.

- Some utilities we missed in the old implementation, including tests.

## Outlook 

Looking back at what we had learned and gained in terms of code, directly
portable into`jpscore`, it became clear that when considering rewriting vs.
refactoring, the latter was often conceived as a try to achieve small gains.
However, we lacked a clear picture of how we ideally would want the application
to be structured.

We are now looking at the existing code with a different view. How can we mold
this into what we want? In retrospect, this question has become much more
straightforward, even if it is still far from simple to answer.

Equipped with this valuable experience, we try to continuously revise the
JuPedSim tools so that we believe they are usable and customizable.

Yes, this implies probably costing more time in the end, but we hope to retain
working software along the way during this time. The first steps in this
direction have been taken with regards to the visualization tool `jpsvis` for
which we hope to have a new release ready soon.

Let see how far we can take this!
