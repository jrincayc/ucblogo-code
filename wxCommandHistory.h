/*
 *      wxCommandHistory.h        wx logo terminal command history module
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WXCOMMANDHISTORY_H_INCLUDED_
#define WXCOMMANDHISTORY_H_INCLUDED_


class wxCommandHistory {

public:
  wxCommandHistory(const int history_size);
  ~wxCommandHistory();

  void handle_command_entered(const char *input_buffer, const int command_len);

  char * handle_previous(const char *input_buffer, const int command_len);
  char * handle_next(const char *input_buffer, const int command_len);

private:
  void maybe_store_working_command(const char *input_buffer, const int command_len);

  // Fixed-size circular buffer for storing command history.
  char **m_command_history;

  // The size of the history buffer (I.E. number of commands to remember).
  int m_history_size;

  // The index to write in to the command history buffer.
  int m_history_in_index;

  // The index to read out from the command history buffer.
  int m_history_out_index;

  // Scratch space to hold working command during history operations.
  char *m_working_command;

  // The depth of navigation in the command history, including the working command (if present).
  int m_history_moves;
};

#endif /* WXCOMMANDHISTORY_H_INCLUDED_ */
