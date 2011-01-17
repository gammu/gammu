:mod:`gammu.worker` - Asynchronous communication to phone.
==========================================================

.. module:: gammu.worker
    :synopsis: Asynchronous communication to phone.

Mostly you should use only :class:`GammuWorker` class, others are only helpers
which are used by this class.


.. class:: GammuCommand(command, params=None, percentage=100)
   :module: gammu.worker

   Storage of single command for gammu.


   .. method:: GammuCommand.get_command()
      :module: gammu.worker

      Returns command name.


   .. method:: GammuCommand.get_params()
      :module: gammu.worker

      Returns command params.


   .. method:: GammuCommand.get_percentage()
      :module: gammu.worker

      Returns percentage of current task.


.. class:: GammuTask(name, commands)
   :module: gammu.worker

   Storage of taks for gammu.


   .. method:: GammuTask.get_name()
      :module: gammu.worker

      Returns task name.


   .. method:: GammuTask.get_next()
      :module: gammu.worker

      Returns next command to be executed as :class:`GammuCommand`.


.. class:: GammuThread(queue, config, callback)
   :module: gammu.worker

   Thread for phone communication.


   .. method:: GammuThread.join(timeout=None)
      :module: gammu.worker

      Terminates thread and waits for it.


   .. method:: GammuThread.kill()
      :module: gammu.worker

      Forces thread end without emptying queue.


   .. method:: GammuThread.run()
      :module: gammu.worker

      Thread body, which handles phone communication. This should not
      be used from outside.


.. class:: GammuWorker(callback)
   :module: gammu.worker

   Wrapper class for asynchronous communication with Gammu. It spaws
   own thread and then passes all commands to this thread. When task is
   done, caller is notified via callback.


   .. method:: GammuWorker.abort()
      :module: gammu.worker

      Aborts any remaining operations.


   .. method:: GammuWorker.configure(config)
      :module: gammu.worker

      Configures gammu instance according to config.

      :param config: Gammu configuration, same as :meth:`gammu.StateMachine.SetConfig` accepts.
      :type config: hash


   .. method:: GammuWorker.enqueue(command, params=None, commands=None)
      :module: gammu.worker

      Enqueues command or task.

      :param command: Command(s) to execute. Each command is tuple containing function name and it's parameters.
      :type command: tuple of list of tuples
      :param params: Parameters to command.
      :type params: tuple or string
      :param commands: List of commands to execute. When this is not none, params are ignored and command is taken as task name.
      :type commands: list of tuples or strings


   .. method:: GammuWorker.enqueue_command(command, params)
      :module: gammu.worker

      Enqueues command.

      :param command: Command(s) to execute. Each command is tuple containing function name and it's parameters.
      :type command: tuple of list of tuples
      :param params: Parameters to command.
      :type params: tuple or string


   .. method:: GammuWorker.enqueue_task(command, commands)
      :module: gammu.worker

      Enqueues task.

      :param command: Command(s) to execute. Each command is tuple containing function name and it's parameters.
      :type command: tuple of list of tuples
      :param commands: List of commands to execute.
      :type commands: list of tuples or strings


   .. method:: GammuWorker.initiate()
      :module: gammu.worker

      Connects to phone.


   .. method:: GammuWorker.terminate(timeout=None)
      :module: gammu.worker

      Terminates phone connection.


.. exception:: InvalidCommand(value)
   :module: gammu.worker

   Exception indicating invalid command.


.. function:: check_worker_command(command)
   :module: gammu.worker

   Checks whether command is valid.

   :param command: Name of command.
   :type command: string


