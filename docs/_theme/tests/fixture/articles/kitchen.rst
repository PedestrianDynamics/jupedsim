.. SPDX-License-Identifier: LGPL-3.0-or-later

.. _kitchen:

Kitchen sink
============

Every markup shape the theme must style, in reStructuredText.

Section level two
-----------------

A paragraph with *emphasis*, **strong**, ``inline code``, a footnote [#f1]_ and an
`external link <https://www.jupedsim.org/>`_.

Section level three
^^^^^^^^^^^^^^^^^^^

Inline math :math:`\rho = N / A` and display math:

.. math::

   v(\rho) = v_0 \left(1 - \frac{\rho}{\rho_{max}}\right)

Admonitions
-----------

.. note:: A note.

.. seealso:: A see-also.

.. hint:: A hint.

.. tip:: A tip.

.. important:: Something important.

.. warning:: A warning.

.. attention:: Attention.

.. caution:: Caution.

.. danger:: Danger.

.. error:: An error.

.. deprecated:: 1.0
   Use something else.

.. versionadded:: 1.2
   Added a thing.

Code
----

.. code-block:: python

   import jupedsim as jps

   def speed(agent):
       return agent.model.desired_speed

.. code-block:: cpp

   #include <vector>

   int main() { std::vector<int> v{1, 2, 3}; return v.size(); }

.. code-block:: bash
   :name: named-block

   echo named

.. code-block:: text

   plain text

.. code-block:: none

   no highlighting

A literal block::

   plain text, no highlighting

A doctest block:

>>> 1 + 1
2

Tables
------

+--------+--------+
| Header | Header |
+========+========+
| cell   | cell   |
+--------+--------+
| cell   | cell   |
+--------+--------+

.. list-table:: A list table
   :header-rows: 1

   * - Model
     - Speed
   * - SocialForce
     - 1.3
   * - CollisionFreeSpeed
     - 1.2

Figure
------

.. figure:: /_static/figure.svg
   :alt: Two agents

   A figure caption.

Definitions
-----------

Agent
   A simulated pedestrian.

Journey
   A path through stages.

Quote
-----

   A plain blockquote, indented once.

.. rubric:: Footnotes

.. [#f1] The footnote text.
