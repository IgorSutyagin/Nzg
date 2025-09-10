////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The nzg software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including nzg as 
//  long as they comply with the license.BSD 2 - Clause License
// 
//  Copyright(c) 2024, TOPCON, All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met :
// 
//  1. Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and /or other materials provided with the distribution.
// 
//  3. The software package includes some companion executive binaries or shared 
//  libraries necessary to execute APs on Windows. These licenses succeed to the 
//  original ones of these software.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// 	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// 	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// 	OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// 	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#pragma once

#include "Tools.h"

namespace nzg
{
    /// A class for recording locations (in the source code) of
    /// exceptions being thrown.
    class ExceptionLocation
    {
    public:
        /**
         * Constructor for location information.
         * @param filename name of source file where exception occurred.
         * @param funcName name of function where exception occurred.
         * @param lineNum line of source file where exception occurred.
         */
        ExceptionLocation(const std::string& filename = std::string(),
            const std::string& funcName = std::string(),
            const unsigned long& lineNum = 0)
            : fileName(filename), functionName(funcName),
            lineNumber(lineNum)
        { }

        /**
         * Destructor.
         */
        ~ExceptionLocation() {}

        /// Accessor for name of source file where exception occurred.
        std::string getFileName() const
        {
            return fileName;
        }
        /// Accessor for name of function where exception occurred.
        std::string getFunctionName() const {
            return functionName;
        }
        /// Accessor for line of source file where exception occurred.
        unsigned long getLineNumber() const {
            return lineNumber;
        }

        /**
         * Debug output function.
         * @param s stream to output debugging information for this class to.
         */
        void dump(std::ostream& s) const {
            s << getFileName() << ":"
#ifdef __FUNCTION__
                << getFunctionName() << ":"
#endif
                << getLineNumber();
        }

        /// Dump to a string
        std::string what() const {
            std::ostringstream oss;
            this->dump(oss);
            return oss.str();
        }

        /**
         * Output stream operator for ::ExceptionLocation.
         * This is intended just to dump all the data in the
         * ::ExceptionLocation to the indicated stream.  \warning Warning: It
         * will _not_ preserve the state of the stream.
         * @param s stream to send ::ExceptionLocation information to.
         * @param e ::ExceptionLocation to "dump".
         * @return a reference to the stream \c s.
         */
        friend std::ostream& operator<<(std::ostream& s, const ExceptionLocation& e);

    private:
        /// Name of source file where exception occurred.
        std::string fileName;
        /// Name of function where exception occurred.
        std::string functionName;
        /// Line in source file where exception occurred.
        unsigned long lineNumber;
    }; // class ExceptionLocation
    
    
    class Exception //: public std::exception
	{
	public:
		/// Exception severity classes.
		enum Severity
		{
			unrecoverable, /**< program can not recover from this exception */
			recoverable    /**< program can recover from this exception */
		};
		Exception() {}
        Exception(const std::string& errorText, const unsigned long& errId = 0, const Severity& sever = unrecoverable) 
            //: std::exception (errorText.c_str())
        {
            text.push_back(errorText);
            errorId = errId;
            severity = sever;
        }
        Exception(const char* errorText, const unsigned long& errId = 0, const Severity& sever = unrecoverable) 
            //: std::exception(errorText)
        {
            text.push_back(std::string(errorText));
            errorId = errId;
            severity = sever;
        }
		Exception(const Exception& e) 
            : errorId(e.errorId),
            locations(e.locations),
            severity(e.severity),
            text(e.text),
            streamBuffer(e.streamBuffer)
        {}

		~Exception() {}

		Exception& operator=(const Exception& e) {
            errorId = e.errorId;
            locations = e.locations;
            severity = e.severity;
            text = e.text;
            // reuse existing stream objects, no matter.
            //streambuf(), ostream((streambuf*)this),
            streamBuffer = e.streamBuffer;

            return *this;
        }


        /// Returns the error ID of the exception.
        unsigned long getErrorId() const
        {
            return errorId;
        };

        /**
         * Sets the error ID to the specified value.
         * @param errId The identifier you want to associate with
         * this error.
         */
        Exception& setErrorId(const unsigned long& errId)
        {
            errorId = errId; return *this;
        };

        /**
         * Adds the location information to the exception object. The
         * library captures this information when an exception is
         * thrown or rethrown. An array of ExceptionLocation objects
         * is stored in the exception object.
         *
         * @param location An IExceptionLocation object containing
         * the following:
         * \li          Function name
         * \li          File name
         * \li          Line number where the function is called
         */
        Exception& addLocation(const ExceptionLocation& location)  {
            locations.push_back(location);
            return *this;
        }

        /**
         * Returns the ExceptionLocation object at the specified index.
         * @param index If the index is not valid, a 0
         * pointer is returned. (well, not really since someone
         * changed all this bah)
         */
        const ExceptionLocation getLocation(const size_t& index = 0) const {
            if (index >= getLocationCount())
            {
                return ExceptionLocation();
            }
            else
            {
                return locations[index];
            }
        }


        /// Returns the number of locations stored in the exception
        /// location array.
        size_t getLocationCount() const {
            return locations.size();
        }

        /**
         * If the thrower (that is, whatever creates the exception)
         * determines the exception is recoverable, 1 is returned. If
         * the thrower determines it is unrecoverable, 0 is returned.
         */
        bool isRecoverable() const {
            return (severity == recoverable);
        }

        /**
         * Sets the severity of the exception.
         * @param sever Use the enumeration Severity to specify
         * the severity of the exception.
         */
        Exception& setSeverity(const Severity& sever) {
            severity = sever; return *this;
        }

        /**
         * Appends the specified text to the text string on the top
         * of the exception text stack.
         * @param errorText The text you want to append.
         */
        Exception& addText(const std::string& errorText) {
            text.push_back(errorText);
            return *this;
        }

        /**
         * Returns an exception text string from the exception text
         * stack.
         *
         * @param index The default index is 0, which is the
         * top of the stack. If you specify an index which is not
         * valid, a 0 pointer is returned.
         */
        std::string getText(const size_t& index = 0) const {
            if (index >= getTextCount())
            {
                std::string tmp;
                return tmp;
            }
            else
            {
                return text[index];
            }
        }


        /// Returns the number of text strings in the exception text stack.
        size_t getTextCount() const {
            return text.size();
        }

        /// Returns the name of the object's class.
        std::string getName() const {
            return "Exception";
        };

        /**
         * Debug output function.
         * @param s stream to output debugging information for this class to.
         */
        void dump(std::ostream& s) const {
            size_t i;
            for (i = 0; i < getTextCount(); i++)
            {
                s << "text " << i << ":" << this->getText(i) << std::endl;
            }
            for (i = 0; i < getLocationCount(); i++)
            {
                s << "location " << i << ":" << getLocation(i).what() << std::endl;
            }
        }


        /// Dump to a string
        std::string what() const {
            std::ostringstream oss;
            this->dump(oss);
            return oss.str();
        }

        /**
         * Output stream operator for ::Exception.
         * This is intended just to dump all the data in the ::Exception to
         * the indicated stream.  \warning Warning:  It will _not_ preserve
         * the state of the stream.
         * @param s stream to send ::Exception information to.
         * @param e ::Exception to "dump".
         * @return a reference to the stream \c s.  */
        friend std::ostream& operator<<(std::ostream& s,
            const Exception& e);

    protected:
        /// Error code.
        unsigned long errorId;
        /// Stack of exception locations (where it was thrown).
        std::vector<ExceptionLocation> locations;
        /// Severity of exception.
        Severity severity;
        /// Text stack describing exception condition.
        std::vector<std::string> text;

        /**
         * This is the streambuf function that actually outputs the
         * data to the device.  Since all output should be done with
         * the standard ostream operators, this function should never
         * be called directly.  In the case of this class, the
         * characters to be output are stored in a buffer and added
         * to the exception text after each newline.
         */
        int overflow(int c) {
            if (c == '\n' || !c)
            {
                if (streamBuffer.length() == 0)
                {
                    return c;
                }
                addText(streamBuffer);
                streamBuffer = "";
                return c;
            }
            streamBuffer.append(1, (char)c);
            return c;
        }


    private:
        /// Buffer for stream output.
        std::string streamBuffer;


	};

	class AntexException : public Exception
	{
	public:
		AntexException(const char* message, int line = 0)
			: Exception(line == 0 ? stringFormat("%s", message) : stringFormat("%s line %i", message, line)) {}

		
	};

	class MatrixException : public Exception
	{
	public:
		MatrixException(const char* message)
			: Exception(message) {}
	};
}

#if defined ( __FUNCTION__ )
#define FILE_LOCATION nzg::ExceptionLocation(__FILE__, __FUNCTION__, __LINE__)
#else
#define FILE_LOCATION nzg::ExceptionLocation(__FILE__, "", __LINE__)
#endif

// For compilers without exceptions, die if you get an exception.
#if defined (NO_EXCEPTIONS_SUPPORT)
/// A macro for adding location when throwing an nzg::Exception
/// @ingroup exceptiongroup
#define GPSTK_THROW(exc) { exc.addLocation(FILE_LOCATION); exc.terminate(); }
/// A macro for adding location when rethrowing an nzg::Exception
/// @ingroup exceptiongroup
#define GPSTK_RETHROW(exc) { exc.addLocation(FILE_LOCATION); exc.terminate(); }
#else
/// A macro for adding location when throwing an nzg::Exception
/// @ingroup exceptiongroup
#define NZG_THROW(exc)   { exc.addLocation(FILE_LOCATION); throw exc; }
/// A macro for adding location when rethrowing an nzg::Exception
/// @ingroup exceptiongroup
#define NZG_RETHROW(exc) { exc.addLocation(FILE_LOCATION); throw; }
#endif

/// Provide an "ASSERT" type macro
#define NZG_ASSERT(CONDITION) if (!(CONDITION)) {                     \
      nzg::AssertionFailure exc("Assertion failed: " #CONDITION);     \
      NZG_THROW(exc);                                                 \
   }

#define NEW_EXCEPTION_CLASS(child, parent) \
class child : public parent  \
{ \
public: \
      /** Default constructor. */ \
   child() : parent() {} \
      /** Copy constructor. */ \
   child(const child& a): parent(a) {} \
      /** Cast constructor. */ \
   child(const nzg::Exception& a) : parent(a) {}; \
      /** \
       * Common use constructor. \
       * @param a text description of exception condition. \
       * @param b error code (default none) \
       * @param c severity of exception (default unrecoverable) \
       */ \
   child(const std::string& a, unsigned long b = 0,\
         nzg::Exception::Severity c = nzg::Exception::unrecoverable) \
         : parent(a, b, c) \
   {};\
      /** \
       * Common use constructor. \
       * @param a text description of exception condition. \
       * @param b error code (default none) \
       * @param c severity of exception (default unrecoverable) \
       */ \
   child(const char* a, unsigned long b = 0,\
   nzg::Exception::Severity c = nzg::Exception::unrecoverable) \
   : parent(a, b, c) \
   {};\
      /** Destructor. */ \
   ~child() {} \
      /** Returns the name of the exception class. */ \
   std::string getName() const {return ( # child);} \
      /** assignment operator for derived exceptions */ \
   child& operator=(const child& kid) \
      { parent::operator=(kid); return *this; } \
      /** ostream operator for derived exceptions */ \
   friend std::ostream& operator<<(std::ostream& s, const child& c) \
      { c.dump(s); return s; } \
}

namespace nzg
{
    /// Thrown when a function is given a parameter value that it invalid
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(InvalidParameter, Exception);

    /// Thrown if a function can not satisfy a request
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(InvalidRequest, Exception);

    /// Thrown when a required condition in a function is not met.
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(AssertionFailure, Exception);

    /// Thrown if a function makes a request of the OS that can't be satisfied.
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(AccessError, Exception);

    /// Attempts to access an "array" or other element that doesn't exist
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(IndexOutOfBoundsException, Exception);

    /// A function was passed an invalid argument
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(InvalidArgumentException, Exception);

    /// Application's configuration is invalid
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(ConfigurationException, Exception);

    /// Attempted to open a file that doesn't exist
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(FileMissingException, Exception);

    /// A problem using a system semaphore
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(SystemSemaphoreException, Exception);

    /// A problem using a system pipe
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(SystemPipeException, Exception);

    /// A problem using a system queue
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(SystemQueueException, Exception);

    /// Unable to allocate memory
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(OutOfMemory, Exception);

    /// Operation failed because it was unable to locate the requested obj
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(ObjectNotFound, AccessError);

    /// Attempted to access a null pointer
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(NullPointerException, Exception);

    /// Attempted to access a unimplemented function
    /// @ingroup exceptiongroup
    NEW_EXCEPTION_CLASS(UnimplementedException, Exception);

    /// FFStreamError is an exception for when the file read doesn't
     /// match the specs for that file type.
     /// @ingroup FileHandling
     /// @ingroup exceptionclass
    NEW_EXCEPTION_CLASS(FFStreamError, nzg::Exception);

    /// This gets thrown if a valid EOF occurs on formattedGetLine.
    /// @ingroup exceptionclass
    NEW_EXCEPTION_CLASS(EndOfFile, nzg::FFStreamError);

    /// Class for StopIterator Python exception, used by Vector.i
    class StopIterator {};

}