***********************
How *JuPedSim* Iterates
***********************

Simulations in *JuPedSim* advance in iterations with uniform time differential.
It is important to understand that *JuPedSim* does not offer semantics such as
"run to completion", rather users are required to call `sim.iterate(...)`
repeatedly and check for their desired end state. 

Between iterations you are free to manipulate simulation state as you
desire. Be aware that there are bounds to what can be set for specific models,
e.g. setting a negative desired speed results in an exception being thrown.
Please review model documentation to see allowed ranges for model parameters.

Iterate deconstructed
=====================

.. mermaid::
   flowchart TD
       start@{ shape: circ, label: "Start" }
	   remove-agents@{ shape: rect, label: "Remove Marked Agents" }
	   update-ns@{ shape: rect, label: "Update Neighbor Search Data Structures" }
	   run-strat-level@{ shape: rect, label: "Update Journeys" }
	   run-tac-level@{ shape: rect, label: "Update Target Location"}
	   run-op-level@{ shape: rect, label: "Update Agents" }
	   advance-clock@{ shape: rect, label: "Advance clock" }
	   stop@{ shape: dbl-circ, label: "Stop" }
	   
	   start --> remove-agents 
	   remove-agents --> update-ns
	   update-ns --> run-strat-level
	   run-strat-level --> run-tac-level
	   run-tac-level --> run-op-level
	   run-op-level --> advance-clock
	   advance-clock --> stop

Agent Removal Explained
-----------------------
TODO:
