:py:mod:`library`
=================

.. py:module:: library


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   library.BuildInfo



Functions
~~~~~~~~~

.. autoapisummary::

   library.set_debug_callback
   library.set_info_callback
   library.set_warning_callback
   library.set_error_callback
   library.get_build_info



.. py:function:: set_debug_callback(fn) -> None

   Set reciever for debug messages.

   Parameters
   ----------
   fn: fn<str>
       function that accepts a msg as string


.. py:function:: set_info_callback(fn) -> None

   Set reciever for info messages.

   Parameters
   ----------
   fn: fn<str>
       function that accepts a msg as string


.. py:function:: set_warning_callback(fn) -> None

   Set reciever for warning messages.

   Parameters
   ----------
   fn: fn<str>
       function that accepts a msg as string


.. py:function:: set_error_callback(fn) -> None

   Set reciever for error messages.

   Parameters
   ----------
   fn: fn<str>
       function that accepts a msg as string


.. py:class:: BuildInfo


   .. py:property:: git_commit_hash
      :type: str


   .. py:property:: git_commit_date
      :type: str


   .. py:property:: git_branch
      :type: str


   .. py:property:: compiler
      :type: str


   .. py:property:: compiler_version
      :type: str


   .. py:property:: library_version
      :type: str



.. py:function:: get_build_info() -> BuildInfo


