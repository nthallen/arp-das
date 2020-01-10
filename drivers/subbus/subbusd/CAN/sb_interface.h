/**
 * \file sb_interface.h
 * Header for a stand-in for the full DAS_IO Interface class
 */
#ifndef SB_INTERFACE_H_INCLUDED
#define SB_INTERFACE_H_INCLUDED
#include <stdint.h>
#include <string>
#include <sys/uio.h>
#include "timeout.h"
  
class Loop;
class Authenticator;
class tm_rcvr;

/**
 */
class sb_interface {
  friend class Authenticator;
  friend class tm_rcvr;
  public:
    sb_interface(const char *name, int bufsz);
    /**
     * @param flag bit-mapped value indicating which event(s) triggered this call.
     * @return true if we should quit
     */
    virtual bool ProcessData(int flag);
    // /** 
     // * Called after interface is added as a child to the Loop.
     // */
    // virtual void adopted();
    // /**
     // * @return a pointer to a Timeout object or zero if no timeout is required.
     // * The default implementation returns a pointer to the TO member.
     // */
    // virtual Timeout *GetTimeout();
    
    /** The file descriptor for this interface */
    int fd;
    
    /** Bit-mapped register to indicate which events this interface is sensitive to. */
    int flags;
    // /** Bit-mapped register to indicate which signals this interface is sensitive to. */
    // uint32_t signals;
    
    // /**
     // * @return true if it terminates.
     // * The default implementation returns true.
     // */
    // virtual bool serialized_signal_handler(uint32_t signals_seen);
    
    static const int Fl_Read = 1;
    static const int Fl_Write = 2;
    static const int Fl_Except = 4;
    static const int Fl_Timeout = 8;
    // /**
     // * Set the threshold to a negative value to report all errors.
     // * The default threshold value is 5.
     // * @param thr New qualified protocol error threshold value
     // */
    // void set_qerr_threshold(int thr);
    // /** The event loop we belong to */
    // Loop *ELoop;
    
    /** getter for interface name. */
    inline const char *get_iname() { return iname; }
    
    // /**
     // * Method to map a global flag number to a bit mask to be
     // * set in a Interface's flags word.
     // * @param gflag_index global flag bit number
     // * @return bit mask selecting the specified global flag.
     // */
    // static inline int gflag(unsigned gflag_index) {
      // return( 1 << (gflag_index+4) );
    // }
    // /**
     // * Increment Interface's ref_count (for use when copying)
     // */
    // inline void reference() { ++ref_count; }
    // /**
     // * Decrement Interface's ref_count and delete if zero;
     // */
    // static void dereference(Interface *P);
    // /**
     // * @param n Expected minimum ref_count value
     // * @return true if ref_count is greater than or equal to n
     // */
    // inline bool ref_check(int n) { return ref_count >= n; }
    // /**
     // * Specifies that reads should use the entire bufsize and
     // * fillbuf() should not add a NUL byte after the read.
     // */
    // inline void set_binary_mode() {
      // binary_offset = 0;
      // binary_mode = true;
    // }
    // /**
     // * @param signum the signal number.
     // * @param handle true to handle the specified signal.
     // * When the signal is observed the serialized_signal_handler()
     // * method will be called for the interface. Must be called
     // * after the sb_interface has been added to the Loop.
     // */
    // void signal(int signum, bool handle = true);
  protected:
    virtual ~sb_interface();
    // /**
     // * Sets up a write of nc bytes from the buffer pointed to by str.
     // * If the write cannot be accomplished immediately, the information
     // * is saved and handled transparently. The caller is
     // * responsible for allocating the output buffer(s) and ensuring
     // * they are not overrun.
     // * 
     // * If an error occurs during writing, iwrite_error(errno) is called,
     // * and it's return value is returned.
     // * @param str Pointer to the output buffer
     // * @param nc The total number of bytes in the output buffer
     // * @param cp The starting offset within the output buffer
     // * @return true if event loop should terminate.
     // */
    // bool iwrite(const char *str, unsigned int nc, unsigned int cp = 0);
    // /**
     // * Sets up a write of the string s by invoking iwrite(str, nc, 0).
     // * There are risks associated with this invocation if the resulting
     // * string is not transmitted immediately. The string argument must
     // * remain unmodified until entirely transmitted.
     // * @param s The string to output
     // * @return true if event loop should terminate.
     // */
    // bool iwrite(const std::string &s);
    // /**
     // * Sets up a write of the string s by invoking iwrite(str, strlen(str), 0).
     // * The string must remain unmodified until the data has been entirely
     // * transmitted.
     // * @param str The string to output
     // * @return true if event loop should terminate.
     // */
    // bool iwrite(const char *str);
    // /**
     // * Sets up a multi-part write
     // * @param iov pointer to an array of iovec structs
     // * @param nparts the number of elements in the iov array
     // * @return true if event loop should terminate.
     // */
    // bool iwritev(struct iovec *iov, int nparts);
    // /**
     // * Cancels a pending iwrite() request, allowing the caller to free
     // * the output buffer.
     // */
    // void iwrite_cancel();
    // /**
     // * @brief Called whenever enqueued data is written to the interface.
     // * @param nb The number of bytes written
     // * @return true if the event loop should terminate.
     // */
    // virtual bool iwritten(int nb);
    // /**
     // * @return true if the error is fatal, false if it has been handled
     // */
    // virtual bool iwrite_error(int my_errno);
    // /**
     // * Internal function to output data
     // * @return true if the event loop should terminate.
     // */
    // bool iwrite_check();
    // /**
     // * @return true if there is no pending data in obuf
     // */
    // inline bool obuf_empty() { return n_wiov == 0; }
    /**
     * Called from fillbuf() when read() returns an error. If read() returns zero,
     * read_error() is called with EOK, which higher-level processors can use
     * to investigate.
     * @param my_errno The errno value or EOK
     * @return true if the error is fatal, false if it has been handled
     */
    virtual bool read_error(int my_errno);
    /**
     * Convert the entire input buffer to printable ASCII, providing
     * appropriate escapes for non-printing characters. Subclasses where
     * ASCII is not relevant can override with appropriate alternatives.
     */
    virtual const char *ascii_escape();
    
    /**
     * Called by ProcessData() when new data is available to the protocol level.
     *
     * protocol_input() and the parsing functions use the sb_interface's buf, cp and nc
     * members to define the current input text. nc is the total number
     * of characters in buf, and cp defines the current offset into
     * buf. Hence the next input character to consider is always buf[cp],
     * and we can only advance while cp < nc.
     *
     * The not_*() parsing functions
     * will advance cp as a side effect. nc is updated by fillbuf()
     * when input is received and by consume() when protocol_input()
     * is finished with input characters. cp is set to zero before
     * protocol_input() is called, so it is important that protocol_input()
     * take care to clear out any characters that have already been fully
     * processed.
     *
     * @return true on an error requiring termination of the driver
     */
    virtual bool protocol_input();
    /**
     * Called by ProcessData() when timeout is reported by select() and
     * this interface has a timeout set, and this interface's timeout
     * has expired. If the interface is also waiting for input, protocol_input()
     * will be called before protocol_timeout(). If the protocol_input()
     * clears the timeout, then protocol_timeout() will not be called.
     *
     * @return true on a condition requiring termination of the driver
     */
    virtual bool protocol_timeout();
    // /**
     // * Called by ProcessData() when an exception occurs.
     // * I have yet to encounter and exceptional condition, so I don't know what to expect.
     // * That may not be true: maybe if a socket gets closed or something?
     // * @return true on a condition requiring termination of the driver
     // */
    // virtual bool protocol_except();
    // /**
     // * Called by ProcessData() when gflag(0) is received.
     // * @return true on a condition requiring termination of the driver
     // */
    // virtual bool tm_sync();
    /**
     * Callback function called when 0 bytes are read from the interface.
     * The default returns true, but this is not appropriate in all
     * situations. Note that process_eof() is not called by close(). It
     * is specifically for addressing the case where the remote 
     * process closes the connection. However, close() is always called
     * before process_eof() is called.
     * 
     * @return true if the event loop should terminate.
     */
    virtual bool process_eof();
    
    /**
     * Shuts down the connection. Note that this function does not
     * invoke process_eof().
     */
    virtual void close();

    /**
     * Updates the input buffer size, reallocating memory as necessary.
     * Failure to allocate the buffer is a fatal error.
     * @param bufsz The size of the input buffer.
     */
    void set_ibufsize(int bufsz);
    
    /**
     * Calls fillbuf(bufsize); If an error is encountered,
     * will return the response from read_error(errno)
     * @return true on an error requiring termination of the driver 
     */
    inline bool fillbuf() { return fillbuf(bufsize); }
    /**
     * @param N The maximum number of characters desired
     * Reads up to N-nc characters into buf. If an error is encountered,
     * will return the response from read_error(errno)
     * @return true on an error requiring termination of the driver 
     */
    bool fillbuf(int N, int flag = Fl_Read);
    /**
     * Remove nchars from the front of buf, and shift the remain
     * characters, if any, to the beginning of the buffer, adjusting
     * cp and nc accordingly
     * Each call to consume() increments the n_empties counter which is
     * reported at termination. If n_fills is much greater than n_empties,
     * you may need to adjust your min and time settings for more efficient
     * operation.
     * The flag argument is used to inform the response to reading 0 bytes.
     * If we are processing a timeout and did not actually see a read flag,
     * then we won't interpret 0 bytes as EOF.
     * @param nchars number of characters to remove from front of buffer
     * @param flag The flag argument from ProcessData().
     */
    void consume(int nchars);
    /**
     * @return true if we are displaying errors.
     */
    bool not_suppressing();
    /**
     * Reports a protocol error message, provided the qualified error count
     * is not above the limit. report_ok() will decrement the qualified
     * error count. It is assumed that all messages are of severity
     * MSG_ERR. Messages at other levels, either more or less severe,
     * should be sent directly to msg().
     */
    void report_err( const char *msg, ... );
    /**
     * Signals that a successful protocol transfer occurred,
     * reducing the qualified error count, potentially reenabling
     * logging of errors.
     * @param nchars Optionally call consume(nchars)
     */
    void report_ok(int nchars = 0);
    /**
     * @brief Searches buf for framing character.
     *
     * Parsing utility function that searches forward in the buffer for the
     * specified framing character. On success, cp is updated to point just
     * past the location of the framing character and the function returns
     * false. If the character is not found, the buffer is emptied and the
     * function returns true.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param c A framing character
     * @return false if the character is found.
     */
    bool not_found(unsigned char c);
    /**
     * @brief Matches a string of hex digits
     *
     * Parsing utility function to read in a hex integer starting
     * at the current position. Integer may be proceeded by optional
     * whitespace.
     *
     * Read the description of protocol_input() to understand how
     * we use buf, cp and nc.
     *
     * These parsing utility functions advance cp, looking for a
     * specific syntactic element (in this case, a hex integer
     * optionally preceded by whitespace). If the element is
     * found, the converted value is stored in the referenced
     * variable (hexval here) and the function will return false.
     *
     * If the function encounters an invalid character or reaches
     * the end of the input text (cp == nc) before the semantic
     * element has been found, the function will return true.
     * If the error occurs before the end of the input buffer,
     * an error will also be reported via report_err().
     *
     * The idea is that a protocol_input() implementation should be
     * able to string together a string of these functions, such as:
     *
     * ```
     * if (not_int(val1) || not_str(",") || not_int(val2) ||
     *     not_str(",") || not_float(flt1)) {
     *   if (cp < nc) {
     *     // input error
     *   }
     *   return false;
     * } else {
     *   // valid input, can reference val1, val2, flt1 
     * }
     * ```
     *
     * Note that failure of a parsing function is only a real error
     * if cp < nc. Otherwise, it probably just means that we are 
     * awaiting more input. How exactly to recover from real input
     * errors is very much dependent on the nature of the input
     * source. If this is a socket connection from another program,
     * it is probably appropriate to consume() all the characters in
     * the input buffer. If the input is coming from a serial device
     * and there is some sort of framing character in the input protocol,
     * it may be reasonable to consume() only up to cp and then search
     * for the framing character using not_found().
     *
     * @param[out] hexval The integer value
     * @return false if a hexadecimal integer was found and converted,
     * true if the current char is not a digit.
     */
    bool not_hex(uint16_t &hexval);
    /**
     * @brief Matches string representing a signed integer
     *
     * Parsing utility function to read in a decimal integer starting
     * at the current position. Integer may be proceeded by optional
     * whitespace and an optional sign.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param[out] val The integer value
     * @return false if an integer was converted, true if the current
     * char is not a digit.
     */
    bool not_int32(int32_t &val );
    /**
     * @brief Matches an unsigned 16-bit integer
     * @param[out] val The converted value
     *
     * This method expects and unsigned value, but will tolerate
     * a leading minus sign, truncating the converted value to 0
     * and issuing an error message.
     *
     * From Zeno_Ser.cc
     * 
     * @return false on a successful match
     */
    bool not_uint16(uint16_t &val);
    /**
     * @brief Matches an integer between 0 and 255
     * @param[out] val The converted value
     *
     * from UserPkts2.cc
     * 
     * @return false on a successful match
     */
    bool not_uint8(uint8_t &val);
    /**
     * @brief Accepts a fixed point integer value
     * @param fix The number of digits after the decimal point
     * @param val The converted value
     * 
     * Accepts an input string of digits with the specified number
     * of digits after the decimal point. The resulting number is
     * converted to an integer by multiplying by 10^fix, which
     * is to say that the fixed point value can be obtained by
     * dividing the returned value by 10^fix.
     *
     * The input string may be preceeded by optional white space
     * and may have a leading negative sign.
     *
     * If no decimal point is observed, or fewer digits after the
     * decimal point than specified are observed, the converted
     * value is scaled correctly.
     *
     * From coelostat.cc
     * 
     * @return false on successful match.
     */
    bool not_fix(int fix, int16_t &val);
    /**
     * @brief Matches a string of binary digits
     * @param word The converted value
     * @param nbits The number of bits to match
     *
     * Accepts optional whitespace followed by the specified number
     * of binary digits. It will fail (returning true) if the
     * specified number of digits are not observed.
     *
     * As written, this routine can successfully match binary strings
     * longer than 16 digits, but the converted value will obviously
     * overflow the word variable.
     *
     * From UPS_parsers.cc
     * 
     * @return false if the specified number of bits are matched
     */
    bool not_bin(int n_bits, uint16_t &word);
    /**
     * @brief Matches a specific number of decimal digits
     * @param n_bits The number of digits
     * @param[out] value The converted value
     *
     * From Zeno_Ser.cc
     * 
     * @return false on a successful match
     */
    bool not_ndigits(int n, int &value);
    /**
     * @brief Matches a specific number of hexadecimal digits
     * @param n_digits The number of digits 1 <= n_digits <= 4
     * @param[out] value The converted value
     *
     * From Zeno_Ser.cc
     * 
     * @return false on a successful match
     */
    bool not_nhexdigits(int n_digits, uint16_t &value);
    /**
     * @brief Check for ISO8601 timestamp
     * 
     * When w_hyphens is false, matches the pattern:
     *  - YYYY MM DDThh:mm:ss.sss
     *
     * When w_hyphens is true, matches the pattern:
     *  - YYYY-MM-DDThh:mm:ss.sss
     *
     * On success, value is assigned the timestamp value converted to
     * unix time (seconds since the epoch 1970-01-01T00:00:00).
     * 
     * Originally from IWG1.cc and UserPkts2.cc.
     *
     * @return false on successful match
     */
    bool not_ISO8601(double &Time, bool w_hyphens = true);
    /**
     * @brief Accepts a string representing a floating point number
     * without converting it.
     *
     * This is used internally by not_float() and not_double() to
     * correctly recognize partial matches at the end of input.
     *
     * @return false if a floating point string is matched.
     */
    bool not_fptext();
    /**
     * @brief Accepts a floating point string and converts it to a float value.
     *
     * Parsing utility function to convert a string in the input
     * buffer to a float value. Updates cp to point just after the
     * converted string on success.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param val[out] The converted float value
     * @return false if the conversion succeeded.
     */
    bool not_float( float &val );
    /**
     * @brief Accepts a floating point string and converts it to a double value.
     *
     * Parsing utility function to convert a string in the input
     * buffer to a double value. Updates cp to point just after the
     * converted string on success.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param val[out] The converted double value
     * @return false if the conversion succeeded.
     */
    bool not_double( double &value );
    /**
     * @brief Match a float or NaN
     *
     * From IWG1.cc and UserPkts2.cc
     * 
     * Accepts a float or an explicit 'NaN' (case insensitive).
     * In the case of NaN, the specified NaNval is used. The
     * NaNval is also used for missing values, as indicated
     * if the next character is a comma or CR.
     *
     * @return false on a successful match
     */
    bool not_nfloat(float *value, float NaNval = 99999.);
    /**
     * @brief Matches a literal string
     *
     * Parsing utility function to check that the specified string matches the
     * input at the current position. On success, advances cp to just
     * after the matched string. On failure, cp points to the first
     * character that does not match, which may be the end of the
     * input buffer.
     *
     * Since both str and len are specified, the specified string may
     * include NUL characters.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param str The comparison string
     * @param len The length of the string
     * @return zero if the string matches the input buffer.
     */
    bool not_str(const char *str, unsigned int len);
    /**
     * @brief Matches a literal string
     *
     * Parsing utility function to check that the specified string matches the
     * input at the current position. On success, advances cp to just
     * after the matched string. On failure, cp points to the first
     * character that does not match, which may be the end of the
     * input buffer.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param s The comparison string
     * @return false if the string matches the input buffer.
     */
    bool not_str(const std::string &s);
    /**
     * @brief Matches a literal string
     *
     * Parsing utility function to check that the specified string matches the
     * input at the current position. On success, advances cp to just
     * after the matched string. On failure, cp points to the first
     * character that does not match, which may be the end of the
     * input buffer.
     *
     * See not_hex() for a more detailed description of these parsing
     * functions.
     *
     * @param str The comparison ASCIIZ string.
     * @return false if the string matches the input buffer.
     */
    bool not_str(const char *str);
    /**
     * @brief Match one of two alternative strings
     * @param alt1 First alternative string
     * @param alt2 Second alternative string
     * @param[out] matched 1 if alt1 matches, 2 if alt2 matches, 0 otherwise
     * @param context A string to specify the input context in error messages
     *
     * Originally from SunRoof.cc.
     *
     * @return false if alt1 or alt2 matches the input.
     */
    bool not_alt(const char *alt1, const char *alt2, int &matched,
                 const char *context);
    /**
     * @brief Matches a single character from a string of alternatives
     * @param alternatives The string of allowed characters
     *
     * Checks the next character and advances if it
     * matches any of the characters in the string of alternatives.
     * 
     * From sq_cmd.cc
     * 
     * @return true if a match was found.
     */
    bool not_any(const char *alternatives);
    /**
     * @brief Matches a word delimited by comma
     * @param KWbuf holds the matched keyword
     *
     * From UserPkts2.cc
     *
     * This is a fairly application-specific test. It accepts:
     *  - optional leading spaces
     *  - up to 30 characters that are not space or comma
     *  - requires a trailing comma
     * 
     * On success, the matched string is copied into KWbuf.
     *
     * To be more broadly useful, this could accept a list of
     * possible delimiters, or simply return the keyword on
     * any character no considered part of a word. It would
     * also make sense to be more specific on what characters
     * are allowed in a word.
     * 
     * @return false on a successful match
     */
    bool not_KW(char *KWbuf);
    /** The name of this interface. Used in diagnostic messages. */
    const char *iname;
    /** The number of characters in buf */
    unsigned int nc;
    /** The current character offset in buf */
    unsigned int cp;
    /** The input buffer */
    unsigned char *buf;
    /** The current size of the input buffer */
    int bufsize;
    int n_fills, n_empties;
    int n_eagain, n_eintr;
    // /** The current output buffer */
    // unsigned char *obuf;
    // /** The number of characters in obuf */
    // unsigned int onc;
    // /** The current character offset in obuf */
    // unsigned int ocp;
    /** Timeout object */
    Timeout TO;
    /** Used to determine read size. Set to -1 for ASCII mode, 0 for binary */
    int binary_offset;
    /** true if we are in binary mode */
    bool binary_mode;
  private:
    /**
     * @brief Look ahead in buf
     * @param str String to match
     * @return 1 if str is matched, 0 if str is partially matched
     * up to the end of input, -1 if characters not matching str
     * are observed.
     */
    int match(const char *str);
    /** The qualified protocol error threshold */
    int qerr_threshold;
    /** Number of qualified protocol errors. Decremented by report_ok() */
    int n_errors;
    /** Number of protocol error messages currently suppressed. */
    int n_suppressed;
    /** Total number of errors found. */
    int total_errors;
    int total_suppressed;
    struct iovec pvt_iov;
    struct iovec *wiov;
    int n_wiov;
    int ref_count;
};

#endif
