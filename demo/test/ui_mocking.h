#pragma once

/// @brief initializes the mocking input and output queues
void io_mock_init (void);

/// @brief destroys the mocking in/output queues
void io_mock_destroy (void);

/// @brief enables printing to stdout
void io_mock_enable_printing (void);

/// @brief disables printing to stdout
void io_mock_disable_printing (void);

/// @brief enables mocking
void io_mock_enable (void);

/// @brief disables mocking
void io_mock_disable (void);

/// @brief removes all previous data in the mocking input queue
void io_mock_reset_input (void);

/// @brief removes all previous data in the mocking output queue
void io_mock_reset_output (void);

/// @brief adds a string for the input mocking
/// @param str the string to read from instead
void io_mock_add_input (char *str);

/// @brief checks the first line written to console without removing it
char *io_mock_peek_output (void);

/// @brief checks the first line written to console AND removes it
/// @attention char * is heap allocated and must be freed using free()
char *io_mock_pop_output (void);

/// @brief returns remaning entries in output queue
size_t io_mock_output_length (void);

/// @brief returns remaning entries in input queue
size_t io_mock_input_length (void);