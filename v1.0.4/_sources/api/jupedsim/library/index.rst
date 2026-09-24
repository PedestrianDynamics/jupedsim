:orphan:

:py:mod:`jupedsim.library`
==========================

.. py:module:: jupedsim.library


Module Contents
---------------

.. py:function:: set_debug_callback(fn: Callable[[str], None]) -> None

   Set receiver for debug messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_info_callback(fn: Callable[[str], None]) -> None

   Set receiver for info messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_warning_callback(fn: Callable[[str], None]) -> None

   Set receiver for warning messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_error_callback(fn: Callable[[str], None]) -> None

   Set receiver for error messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:class:: BuildInfo


   .. py:property:: git_commit_hash
      :type: str

      SHA1 commit hash of this version.

      :returns: SHA1 of this version.

   .. py:property:: git_commit_date
      :type: str

      Date this commit was created.

      :returns: Date the commit of this version as string.

   .. py:property:: git_branch
      :type: str

      Branch this commit was crated from.

      :returns: name of the branch this version was build from.

   .. py:property:: compiler
      :type: str

      Compiler the native code was compiled with.

      :returns: Compiler identification.

   .. py:property:: compiler_version
      :type: str

      Compiler version the native code was compiled with.

      :returns: Compiler version number.

   .. py:property:: library_version
      :type: str



.. py:function:: get_build_info() -> BuildInfo

   Get build information about jupedsim.

   The received :class:`BuildInfo` is printable, e.g.

   .. code:: python

       print(get_build_info())

   This will display a human-readable string stating
   basic information about this library.


