Porting from libGammu older than 1.12.0
========================================

Rationale for API change
------------------------

This document describes what you have to change in your code, if you
used Gammu older than 1.12.0. This release came with huge changes to
API, which has to be done for various reasons:

- ABI stability. - Till now almost every change in internals of any
  driver lead to ABI change. If we would correctly increase soname on
  each ABI change, we would be somewhere near 200, what is not something
  we could be proud of.

- Centralisation of variables cleanup. - Currently all phone drivers
  have to do some common things in each function. New API allows one to
  centralize those operations in one place.

- Exposing of internals. - Old API exposed too much of Gammu internals,
  what could be misused by programmers and could lead to unexpected
  behaviour when some internals are changed.

Changes you have to do in your code
-----------------------------------

Bellow examples expect ``sm`` to be state machine structure in your current
code, change it to appropriate variable name if it differs.

1. Use pointer to :c:type:`GSM_StateMachine` instead of it. API now do not expose
   this structure, so you will get compiler error. You should allocate
   this pointer by :c:func:`GSM_AllocStateMachine` and free by
   :c:func:`GSM_FreeStateMachine`.

2. Change all phone functions from ``sm.Phone.Functions->SomeFunction`` to
   ``GSM_SomeFunction``. Only functions which results were stored inside
   state machine structure have changed signature to include results of
   the operation.

3. All callbacks are set by function ``GSM_Set*Callback`` instead of
   directly accessing structure.

4. Some function have been renamed to follow ``GSM_*`` naming conventions.

As there might be some functions still missing from new API, don't
hesitate to contact author or ask on mailing list if you miss something.

API documentation can be generated using Doxygen (make apidoc in build
tree) or Sphinx and is part of this manual.

.. seealso:: :ref:`libgammu`
