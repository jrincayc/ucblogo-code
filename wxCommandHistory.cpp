/*
 *      wxCommandHistory.cpp      wx logo terminal command history module
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

#include <cstddef>
#include <cstring>

#include "wxCommandHistory.h"

char * clone_string_segment(const char *src_string, const int len);


wxCommandHistory::wxCommandHistory(const int history_size) {
  // Set up the buffer for storing commands.
  m_history_size = history_size;

  m_command_history = new char *[m_history_size];
  memset(m_command_history, (int)NULL, sizeof(char *) * m_history_size);

  m_history_in_index = m_history_out_index = 0;

  // Set up the scratch space for storing the current working command.
  m_working_command = NULL;

  m_history_moves = 0;
}

wxCommandHistory::~wxCommandHistory() {
  for (int i=0; i<m_history_size; i++) {
    if (m_command_history[i] != NULL) {
      delete[] m_command_history[i];
    }
  }
  delete[] m_command_history;

  if (m_working_command != NULL) {
    delete[] m_working_command;
  }
}

void wxCommandHistory::handle_command_entered(const char *input_buffer, const int command_len) {

  // Clear the current working command scratch space.
  m_history_moves = 0;
  if (m_working_command != NULL) {
    delete[] m_working_command;
    m_working_command = NULL;
  }

  // Guard against storing empty commands.
  if (command_len < 1) {
    return;
  }

  // Store the command in the history.
  m_command_history[m_history_in_index] = clone_string_segment(input_buffer, command_len);
  m_history_in_index++;

  // Handle buffer wraparound.
  if (m_history_in_index >= m_history_size) {
    m_history_in_index = 0;
  }

  // Free up space for the next command if needed.
  if (m_command_history[m_history_in_index] != NULL) {
    delete[] m_command_history[m_history_in_index];
    m_command_history[m_history_in_index] = NULL;
  }

  // Update the output index.
  m_history_out_index = m_history_in_index;
}

char * wxCommandHistory::handle_previous(const char *input_buffer, const int command_len) {
  maybe_store_working_command(input_buffer, command_len);

  // If at the beginning of history navigation, and a scratch command has been stored, return it.
  if (m_working_command != NULL && m_history_moves == 0) {
    m_history_moves++;

    return m_working_command;
  }

  int old_history_out_index = m_history_out_index;

  m_history_out_index--;

  // Handle buffer wraparound.
  if (m_history_out_index < 0) {
    m_history_out_index = m_history_size - 1;
  }

  if (m_command_history[m_history_out_index] != NULL) {
    // Return the command from history.
    m_history_moves++;

    return m_command_history[m_history_out_index];
  } else {
    // There is no more history, restore the history out index.
    m_history_out_index = old_history_out_index;

    return NULL;
  }
}

char * wxCommandHistory::handle_next(const char *input_buffer, const int command_len) {
  maybe_store_working_command(input_buffer, command_len);

  if (m_command_history[m_history_out_index] != NULL) {
    m_history_out_index++;
  }

  // Handle buffer wraparound.
  if (m_history_out_index >= m_history_size) {
    m_history_out_index = 0;
  }

  if (m_history_moves > 0) {
    m_history_moves--;
  }

  // If at the beginning of history navigation, and a scratch command has been stored, return it.
  if (m_command_history[m_history_out_index] == NULL) {

    if (m_working_command != NULL && m_history_moves == 1) {
      return m_working_command;
    }
  }

  // Return the command or NULL to signify there are no more commands in history.
  return m_command_history[m_history_out_index];
}

void wxCommandHistory::maybe_store_working_command(const char *input_buffer, const int command_len) {

  // Guard against storing empty commands.
  if (command_len < 1) {
    return;
  }

  if (m_history_moves == 0 || (m_history_moves == 1 && m_working_command != NULL)) {
    // Store the current working command in the command scratch space.
    if (m_working_command != NULL) {
      delete[] m_working_command;
    }
    m_working_command = clone_string_segment(input_buffer, command_len);

    // Only increment when first saving a working command, leave move count
    // as-is when updating the command scratch space.
    if (m_history_moves == 0) {
      m_history_moves++;
    }
  }
}

char * clone_string_segment(const char *src_string, const int len) {
  char *cloned_string = new char[len + 1];

  for(int i = 0; i < len; i++) {
    cloned_string[i] = src_string[i];
  }
  cloned_string[len] = '\0';

  return cloned_string;
}
