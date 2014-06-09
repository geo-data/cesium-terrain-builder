#ifndef COMMANDER_HPP
#define COMMANDER_HPP

/**
 * commander.hpp - a basic C++ wrapper for <https://github.com/clibs/commander>
 *
 * See the `test` directory for an example (including
 * `Makefile`). Documentation is inline and should be read in conjunction to
 * the C commander documentation.
 */

#include <vector>

extern "C" {
#include "commander.h"
}

/**
 * Command class
 *
 * This wraps the C `command_t` structure, providing memory managment for it as
 * well as setting the value of `command->data` to be a pointer to the class:
 * this allows subclasses of `Command` to transparently pass around data
 * between `command_option` callbacks.
 */
class Command {
public:
  Command(const char *name, const char *version) {
    command = new command_t();
    command_init(command, name, version);
    command->data = static_cast<void *>(this); // allow callbacks to reference this class
  }

  /**
   * Free up command related resources
   */
  ~Command() {
    command_free(command);
    delete command;
  }

  /**
   * Print the help message and exit
   */
  void
  help() const {
    command_help(command);
  }

  /**
   * Wrap `command_option`
   *
   * The callback will contain a reference to 
   */
  void 
  option(const char *shortOpt, const char *longOpt, const char *usage,
         command_callback_t cb) {
    command_option(command, shortOpt, longOpt, usage, cb);
  }

  /**
   * Wrap `command_parse`
   */
  void
  parse(int argc, char *argv[]) {
    command_parse(command, argc, argv);
  }

  /**
   * Access any additional arguments as a vector list
   */
  std::vector<const char *>
  additionalArgs() const {
    std::vector<const char *> args(command->argc);
    args.assign(command->argv, command->argv + command->argc);
    return args;
  }

  /**
   * Set the usage text, returning the previous value
   */
  const char *
  setUsage(const char *usage) {
    const char *previousUsage = command->usage;
    command->usage = usage;
    return previousUsage;
  }

  /**
   * Get the usage text
   */
  const char *
  getUsage() const {
    return command->usage;
  }

protected:
  /**
   * Retrieve the `Command` instance associated with a `command_t` struct
   *
   * This is used in option callbacks manipulate the associated `Command`
   * instance.
   */
  static Command *
  self(command_t *command) {
    return static_cast<Command *>(command->data);
  }

  command_t *command;           // the C struct we are wrapping
};

#endif /* COMMANDER_HPP */
