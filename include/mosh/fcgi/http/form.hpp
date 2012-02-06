//! @file  mosh/fcgi/http/form.hpp HTTP Session form data
/*!*************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of mosh-fcgi.                                          *
*                                                                          *
* mosh-fcgi is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#ifndef MOSH_FCGI_HTTP_FORM_HPP
#define MOSH_FCGI_HTTP_FORM_HPP

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

extern "C" {
// dir check and getpid
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
}

#include <mosh/fcgi/bits/cmp.hpp>
#include <mosh/fcgi/boyer_moore.hpp>
#ifdef MOSH_FCGI_USE_HYBRID_VECTOR
#define VECOPS_HV
#include <hybrid_vector>
#endif
#ifdef MOSH_FCGI_USE_STXXL
#define VECOPS_STXXL
#include <stxxl/vector>
#endif
#include <mosh/fcgi/bits/hash.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/bits/namespace.hpp>


MOSH_FCGI_BEGIN

namespace http {
/*! @brief Structures for HTML form data
 * @code
 * List of classes:
 *
 * Classname       | multiple values | headers | files
 * -----------------------------------------------------
 *   Entry          |     yes         |    no   |   no
 *   MP_entry       |     no          |    yes  |   yes
 *   MP_mixed_entry |     yes         |    yes  |   yes
 * @endcode
 */
namespace form {


/*! @brief Type of object of subclass.
 *
 * Data<T> uses the value of this object to compare if the derived parts
 * are comparable.
 */
enum class Type {
	form_entry, //!< Subclass is Form_entry
	mp_entry, //!< Subclass is MP_entry
	mixed_entry //!< Subclass is MP_mixed_entry
};

/*! @brief Base class for form objects.
 *
 * This class stores the name of the form element and the type of the form
 * element (@see Type).
 *
 *
 * @remark Comparison operators throw std::runtime_error if the types are unequal
 *
 * @tparam char_type character type
 */
template <class char_type>
class Data {
private:
	typedef Data<char_type> this_type;
protected:
	/*! @brief Create a new form data key of a given type.
	 * @param f type of object
	 */
	Data(Type f) : type(f)	{ }
	
	/*! @brief Create a new form data key of a given type, with a given name.
	 * @param f type of object
	 * @param[in] name name of form element
	 */
	Data(Type f, const std::basic_string<char_type>& name)
	: type(f), name(name)
	{ }

	/*! @brief Create a new form data key of a given type, with a given name.
	 * @param f type of object
	 * @param[in] name name of form element
	 */
	Data(Type f, std::basic_string<char_type>&& name)
	: type(f), name(std::move(name))
	{ }

	/*! @brief Create a new form data key by copying the existing one.
	 * Type must be specified.
	 * @param f type of object
	 * @param[in] entry entry to copy values from
	 */
	Data(Type f, const this_type& entry)
	: type(f), name(entry.name)
	{ }
	
	/*! @brief Create a new form data key by moving values from the existing one.
	 * Type must be specified.
	 * @param f type of object
	 * @param[in] entry entry to move values from
	 */
	Data(Type f, this_type&& entry)
	: type(f), name(std::move(entry.name))
	{ }

	virtual ~Data()
	{ }

	/*! @brief Assignment operator for base class members
	 *  @throws std::runtime_error if the derived classes aren't the same
	 */
	this_type& operator = (const this_type& dat) {
		if (type != dat.type)
			throw std::runtime_error("caught attempt to use base type of incompatible derived types");
		if (this != &dat) {
			name = dat.name;
		}
		return *this;
	}

	/*! @brief Assignment operator for base class members
	 *  @throws std::runtime_error if the derived classes aren't the same
	 */
	this_type& operator = (this_type&& dat) {
		if (type != dat.type)
			throw std::runtime_error("caught attempt to use base type of incompatible derived types");
		if (this != &dat) {
			name = std::move(dat.name);
		}
		return *this;
	}

	inline bool operator == (const this_type& dat) const { return cmp(dat, Cmp_test::eq); }
	inline bool operator != (const this_type& dat) const { return cmp(dat, Cmp_test::ne); }
	inline bool operator < (const this_type& dat) const { return cmp(dat, Cmp_test::lt); }
	inline bool operator <= (const this_type& dat) const { return cmp(dat, Cmp_test::le); }
	inline bool operator >= (const this_type& dat) const { return cmp(dat, Cmp_test::ge); }
	inline bool operator > (const this_type& dat) const { return cmp(dat, Cmp_test::gt); }

	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return name.empty();
	}
	//! cmp operator to generalize comparisons
	bool cmp(const this_type& dat, Cmp_test test) const {
		if ((test == Cmp_test::eq || test == Cmp_test::le || test == Cmp_test::gt) && this == &dat)
			return true;
		if (type != dat.type)
			throw std::runtime_error("caught attempt to compare base type of incompatible derived types");
		return ::MOSH_FCGI::cmp(name, dat.name, test);
	}
private:
	Type type; // should be const

public:
	//! entry name
	std::basic_string<char_type> name;

};

/*! @brief Form entry
 *
 * The usual form entry, this class is appropriate for holding GETs and x-www-formurl-encoded
 * vars. To allow for duplicate values, an internal vector is employed.
 *
 * @tparam char_type type of char to use in strings
 * @tparam value_type value type
 */
template <class char_type, class value_type = std::basic_string<char_type>>
class Entry : public Data<char_type> {
private:
	typedef Entry<char_type, value_type> this_type;
	typedef Data<char_type> base_type;
public:
	Entry()	: base_type(Type::form_entry) { }
	/*! @brief Create a form entry with a given name and value
	 * @param[in] name entry name
	 * @param[in] value initial value
	 */
	Entry(const std::basic_string<char_type>& name, value_type&& value = value_type())
	: base_type(Type::form_entry, name)
	{
		add_value(value);
	}
	/*! @brief Create a form entry with a given name and value
	 * @param[in] name entry name
	 * @param[in] value initial value
	 */
	Entry(const std::basic_string<char_type>& name, const value_type& value)
	: base_type(Type::form_entry, name)
	{
		add_value(value);
	}
	//! Move ctor
	Entry(this_type&& e)
	: base_type(Type::form_entry, e), values(std::move(e.values))
	{
		values = std::move(e.values);
	}

	virtual ~Entry()
	{ }
	
	/*! @brief Add a value to the values list
	 * This add_s a value to the values list.
	 * If uniqueness is enabled, std::find checks for duplicates, making insertion
	 * O(n). Otherwise, it is O(1).
	 * @param[in] value the value to add
	 */
	void add_value(value_type&& value) {
		if ((!uniqueness_mode)
		|| (std::find(values.begin(), values.end(), std::move(value)) == values.end()))
			values.push_back(std::move(value));
	}
	
	//! Enables uniqueness mode
	void enable_unique_mode() {
		uniqueness_mode = 1;
	}
	//! Disables uniqueness mode
	void disable_unique_mode() {
		uniqueness_mode = 0;
	}
	
	/*! @brief Get a ref to the last value
	 * @throws std::runtime_error if the vector is empty
	 */
	value_type& last_value() {
		if (values.empty())
			throw std::runtime_error("undefined operation");
		return values.back();
	}

	/*! @brief Get the value
	 * @throws std::runtime_error if the vector has multiple elements (use values() instead)
	 * @returns the first (and only) value or value_type's default value if the vec is empty
	 */
	const value_type& value() const {
		static value_type _v;
		if (values.size() > 1)
			throw std::runtime_error("undefined operation");
		else if (values.empty())
			return _v;
		else
			return values.front();
	}

	//! Checks if there are no values
	bool is_empty_value() const {
		return (values.empty());
	}
	/*! @brief Checks if there is only one value
	 * Note: an empty list of values counts as scalar
	 */
	bool is_scalar_value() const {
		return (values.size() <= 1); // strictly speaking, an empty value is a scalar value
	}

	this_type& operator = (this_type&& e) {
		if (this != &e) {
			base_type::operator = (std::move(e));
			values = std::move(e.values);
		}
		return *this;
	}
	
	/*! @brief Left-shift operator
	 * This is used to shift in an additional value to the end of the vec.
	 * @param[in] val the value to add
	 * @returns *this
	 */
	inline this_type& operator << (value_type&& val) {
		add_value(std::move(val));
		return *this;
	}

	inline bool operator == (const this_type& e) const { return this->cmp(e, Cmp_test::eq); }
	inline bool operator != (const this_type& e) const { return this->cmp(e, Cmp_test::ne); }
	inline bool operator < (const this_type& e) const { return this->cmp(e, Cmp_test::lt); }
	inline bool operator <= (const this_type& e) const { return this->cmp(e, Cmp_test::le); }
	inline bool operator >= (const this_type& e) const { return this->cmp(e, Cmp_test::ge); }
	inline bool operator > (const this_type& e) const { return this->cmp(e, Cmp_test::gt); }
	
	//! List of values for this particular entry
	std::vector<value_type> values;
protected:
	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return (base_type::am_i_empty() && values.empty());
	}
	//! cmp operator to generalize comparisons
	bool cmp(const this_type& e, Cmp_test test) const {
		if ((test == Cmp_test::eq || test == Cmp_test::le || test == Cmp_test::gt) && this == &e)
			return true;
		if (!base_type::cmp(e, test))
			return false;
		return true;
	}
private:
	bool uniqueness_mode;
	
	// enforce noncopy semantics
	Entry(this_type const&) = delete;
	this_type& operator = (this_type const&) = delete;
	this_type& operator << (value_type const&) = delete;
	void add_value(value_type const&) = delete;

};

/*! @brief Multipart entry
 *
 * Holds a multipart/form-data element.
 *
 * @tparam char_type type of char to use in strings
 * @tparam buffer_type type of value buffer (used for file data)
 */
template <class char_type, class value_type = std::basic_string<char_type>>
class MP_entry : public Data<char_type> {
private:
	typedef MP_entry<char_type, value_type> this_type;
	typedef Data<char_type> base_type;
	typedef std::basic_string<char_type> string_type;
// Not all libc++'s have C++11 fstream semantics, so we use the unique_ptr hack
// to implement move semantics if configure deems it necessary
#ifdef HAVE_CXX11_IOSTREAM
	typedef std::fstream file_type;
#else
	typedef std::unique_ptr<std::fstream> file_type;
#endif
	enum class Mode {
		entry,
		file
	};
public:
	MP_entry() : base_type(Type::mp_entry) { }
	
	/*! @brief Create a new MP_entry
	 * @param[in] name entry name
	 * @param[in] filename entry filename
	 * @param[in] content_type content type
	 */
	MP_entry(const string_type& name, const string_type& filename = string_type(),
		const std::string& content_type = "")
	: base_type(Type::mp_entry, name), filename(filename), content_type(content_type)
	{
		if (filename.empty()) {
			mode = Mode::entry;
		} else {
			mode = Mode::file;
			actual_filename = ""; // leave uninitialized
		}
	}

private:
	// This class is noncopyable
	MP_entry(const this_type&) = delete;
public:
	//! Move constructor
	MP_entry(this_type&& mpe)
	: base_type(Type::mp_entry, std::move(mpe)), filename(std::move(mpe.filename)),
		content_type(std::move(mpe.content_type)), headers(std::move(mpe.headers))
	{
		if (filename.empty()) {
			mode = Mode::entry;
			_data = std::move(mpe._data);
		} else {
			mode = Mode::file;
			actual_filename = std::move(mpe.actual_filename);
			_file = std::move(mpe._file);
		}
	}

	virtual ~MP_entry() {
		if (is_file()) {
			if (!f_persist && !actual_filename.empty()) {
				errno = 0;
				if (unlink(actual_filename.c_str()) == -1) {
					propagate_error(_eh_fl_::call_eh, "unlink()", errno);
				}
			}
		}
	}

	/*! @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 -*/
	void add_header(const std::string& k, const std::string& v) {
		headers.insert(std::make_pair(k, v));
	}

	/*! @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 */
	void add_header(std::string&& k, std::string& v) {
		headers.insert(std::make_pair(std::move(k), std::move(v)));
	}

	/*! @brief Add data
	 * @param[in] s data
	 */
	void append_text(std::basic_string<char_type>&& s) {
		append_text(&(*s.begin()), &(*s.end()));
	}
	/*! @brief Add data
	 * @param[in] s start of data
	 * @param[in] e end of data
	 */
	void append_text(const char_type* s, const char_type* e) {
		require_entry_mode();
		_data.insert(_data.end(), s, e);
	}

	/*! @brief Add bytes
	 * @param[in] s data
	 */
	void append_binary(std::string&& s) {
		append_binary(&(*s.begin()), &(*s.end()));
	}
	/*! @brief Add bytes
	 * @param[in] s start of data
	 * @param[in] e end of data
	 */
	void append_binary(const char* s, const char* e) {
		require_file_mode();
		if (actual_filename.empty()) {
			using namespace std;

			actual_filename = make_filename();
#ifdef HAVE_CXX11_IOSTREAM
			_file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
			_file.open(actual_filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
#else
			_file.reset(new std::fstream);
			_file->exceptions(std::ios_base::failbit | std::ios_base::badbit);
			_file->open(actual_filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
#endif
		}
#ifdef HAVE_CXX11_IOSTREAM
		_file.write(s, e - s);
#else
		_file->write(s, e - s);
#endif
	}

	//! Get the file size
	std::fstream::off_type filesize() const {

		require_file_mode();
		if (!actual_filename.empty())
#ifdef HAVE_CXX11_IOSTREAM
			return _file.tellp();
#else
			return _file->tellp();
#endif
		else
			return 0;
	}

	bool is_entry() const { return mode == Mode::entry; }
	bool is_file() const { return mode == Mode::file; }
	
	const std::string& disk_filename() const {
		require_file_mode();
		return actual_filename;
	}
	const value_type& data() const {
		require_entry_mode();
		return _data;
	}
	
	std::ifstream& file() {
		require_file_mode();
#if HAVE_CXX11_IOSTREAM
		_file.sync();
		return _file;
#else
		_file->sync();
		return *_file;
#endif
	}

	void make_file_persistent() const {
		require_file_mode();
		f_persist = true;
	}

private:
	// This class is noncopyable
	this_type& operator = (const this_type&) = delete;
public:
	this_type& operator = (this_type&& mpe) {
		if (this != &mpe) {
			base_type::operator = (std::move(mpe));
			filename = std::move(mpe.filename);
			content_type = std::move(mpe.content_type);
			headers = std::move(mpe.headers);
			mode = std::move(mpe.mode);
			switch (mode) {
			case Mode::entry:
				_data = std::move(mpe._data);
				break;
			case Mode::file:
				actual_filename = std::move(mpe.actual_filename);
				_file = std::move(mpe._file);
				break;
			}
		}
		return *this;
	}

	inline bool operator == (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::eq); }
	inline bool operator != (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::ne); }
	inline bool operator < (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::lt); }
	inline bool operator <= (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::le); }
	inline bool operator >= (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::ge); }
	inline bool operator > (const this_type& mpe) const { return this->cmp(mpe, Cmp_test::gt); }

	//! the file name; if empty, then data is form input instead of file input
	std::basic_string<char_type> filename;
	//! value of Content-Type header
	std::string content_type;
	//! charset
	std::string charset;
	//! Content-Transfer-Encoding (this field is not used internally)
	std::string ct_encoding;	
	//! list of all headers
	std::map<std::string, std::string> headers;
	//! error handler (used for propagating errors of POSIX file functions)
	std::function<void(std::string const&, int)> error_handler;	
protected:
	bool am_i_empty() const {
		return (base_type::am_i_empty()
			&& (mode == Mode::file ? filename.empty() : true)
			&& content_type.empty()
			&& headers.empty()
		);
	}

	//! cmp operator to generalize comparisons
	bool cmp(const this_type& mpe, Cmp_test test) const {
		if ((test == Cmp_test::eq || test == Cmp_test::le || test == Cmp_test::gt) && this == &mpe)
			return true;
		if (!base_type::cmp(mpe, test))
			return false;
		if (!::MOSH_FCGI::cmp(filename, mpe.filename, test))
			return false;
		if (!::MOSH_FCGI::cmp(content_type, mpe.content_type, test))
			return false;
		if (!::MOSH_FCGI::cmp(content_type, mpe.content_type, test))
			return false;
		if (!::MOSH_FCGI::cmp(headers, mpe.headers, test))
			return false;
		return true;
	}

	/*! @brief Propagate an error.
	 *
	 * Only use this (instead of just throwing) when the error is not the user's fault (e.g. I/O error,
	 * 	allocation error)
	 *
	 * @param emsg Error string
	 * @param eno Error number (undefined behavior if there's no constant in &lt;cerrno&gt; corresponding
	 * 		to eno
	 * @see _eh_fl_
	 */
	void propagate_error(std::string const& emsg, int eno) {
		(void)(error_handler ? error_handler(emsg, eno) : throw std::runtime_error("Error handler not defined"));
	}
private:
	value_type _data;
	file_type _file;
	std::string actual_filename;
	Mode mode;
	// Make file persistent (no unlink in dtor)
	mutable bool f_persist;

	// Create a filename in the form of /tmp/mosh-fcgi/$(hostname).$(pid)-$(date.date).$(date.time)-$(sha1(headers))
	std::string make_filename() const { 
		std::string dir = "/tmp/mosh-fcgi/";
		std::string file;
	
		// Check if /tmp/mosh-fcgi exists
		// If it doesn't create it
		// (Note: only mosh-fcgi is created, as posix systems already have /tmp)
		{
			errno = 0;
			DIR* dir = opendir("/tmp/mosh-fcgi");
			if (dir != NULL) {
				closedir(dir);
			} else {
				if (errno == ENOENT) {
					if (mkdir("/tmp/mosh-fcgi", 0700) == -1)
						propagate_error("mkdir()", errno);

				} else {
						propagate_error("opendir()", errno);
				}
			}
		}
		
		file += hostname();
		
		file += "."; 

		// Get pid
		{
			// Get the PID as a string
			std::stringstream s;
			s << getpid();
			file += s.str();
		}
		
		file += "-";
		
		file += time_to_string("%Y%m%d.%H%M%S%f");
		
		file += "-";
		
		// Get sha1 of all headers
		{
			HASHContext* hash = HASH_Create(HASH_AlgSHA1);
			hash::hash(hash, headers);
			hash::hash(hash, filename);
			hash::hash(hash, content_type);
			auto h = hash::hash_finalize(hash);
			HASH_Destroy(hash);

			// Print the values of each byte in h in %02x format
			std::stringstream ss;
			for (auto& i : h)
				ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(i);
			file += ss.str();
		}
		/* Filename components have a maximum length of 255.
		 * If we exceeded that, resize the string so that the length is 255.
		 */
		if (file.size() > 255)
			file.resize(255);
		return dir + file;
	}

	inline void require_entry_mode() const {
		if (mode != Mode::entry)
			throw std::runtime_error("MP_entry is a file");
	}
	
	inline void require_file_mode() const {
		if (mode != Mode::file)
			throw std::runtime_error("MP_entry is an entry");
	}
};

/*! @brief Multipart/mixed entry
 *
 * This class holds (a) multipart/mixed entry(/ies), which is a bunch of headers followed
 * by multipart/form-data fields
 * @tparam char_type type of char to use in strings
 * @tparam buffer_type type of buffer to use to hold file data. If the type
 * 			doesn't exist in bits/vecops.hpp, make a VECOPSSPEC
 * 			for the new vector type.
 * 			Currently available options are std::vector, stxxl::vector,
 * 			and hybid_vector.
 */
template <class char_type, class value_type = std::basic_string<char_type>>
class MP_mixed_entry : public Data<char_type> {
private:
	typedef MP_mixed_entry<char_type, value_type> this_type;
	typedef MP_entry<char_type, value_type> entry_type;
	typedef Data<char_type> base_type;
public:
	MP_mixed_entry() : base_type(Type::mixed_entry) { }
	
	/*! @brief Create a new MP_mixed_entry
	 * @param[in] name entry name
	 * @param[in] boundary boundary string
	 */
	MP_mixed_entry(const std::basic_string<char_type>& name, const std::string& boundary)
	: base_type(Type::mixed_entry, name), bound(boundary)
	{
		s_bound = Boyer_moore_searcher(bound);
	}

private:
	MP_mixed_entry(const this_type&) = delete;
public:
	MP_mixed_entry(this_type&& mme)	
		: base_type(Type::mixed_entry, mme), headers(std::move(mme.headers)),
		values(std::move(mme.values)), bound(mme.bound), s_bound(std::move(mme.s_bound))
	{ }
	//! @brief Create a new MP_mixed_entry from an existing MP_entry
	MP_mixed_entry(entry_type&& mpe)
		: base_type(Type::mixed_entry, std::move(mpe.name)), headers(std::move(mpe.headers))
	{ }

	virtual ~MP_mixed_entry()
	{ }


	/*! @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 */
	void add_header(const std::string& k, const std::string& v) {
		headers.insert(std::make_pair(k, v));
	}

	/*! @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 */
	void add_header(std::string&& k, std::string&& v) {
		headers.insert(std::make_pair(std::move(k), std::move(v)));
	}
	/*! @brief Add a value to the values list
	 * This add_s a value to the values list.
	 * If uniqueness is enabled, std::find checks for duplicates, making insertion
	 * O(n). Otherwise, it is O(1).
	 * @param[in] value the value to add
	 */
	void add_value(entry_type&& value) {
		if ((!uniqueness_mode)
				|| (std::find(values.begin(), values.end(), value) == values.end()))
			values.push_back(std::move(value));
			values.back().error_handler = std::bind(&MP_mixed_entry::propagate_error,
								std::ref(*this), values.back().filename, 
								std::placeholders::_2, std::placeholders::_3);
	}

	/*! @brief sets the boundary
	 * Sets the boundary string and (re-)initializes the searcher
	 * @param[in] s the new boundary string
	 * @returns *this
	 */
	this_type& set_boundary(std::string&& s) {
		bound = std::move(s);
		s_bound = Boyer_moore_searcher(bound);
		return *this;
	}

	/*! @brief sets the boundary
	 * Sets the boundary string and (re-)initializes the searcher
	 * @param[in] s the new boundary string
	 * @returns *this
	 */
	this_type& set_boundary(const std::string& s) {
		bound = s;
		s_bound = Boyer_moore_searcher(bound);
		return *this;

	}

	const std::string& boundary() const {
		return bound;
	}

	//! get a cref to the boundary string
	const Boyer_moore_searcher& boundary_searcher() const {
		return s_bound;
	}

	//! Enables uniqueness mode
	void enable_unique_mode() {
		uniqueness_mode = 1;
	}
	//! Disables uniqueness mode
	void disable_unique_mode() {
		uniqueness_mode = 0;
	}

	/*! @brief Left-shift operator
	 * This is used to shift in an additional value to the end of the vec.
	 * @param[in] val the value to add
	 * @returns *this
	 */
	inline this_type& operator << (value_type&& val) {
		add_value(std::move(val));
		return *this;
	}

	/*! @brief Get the last value
	 * @throws std::runtime_error if the vector is empty
	 */
	entry_type& last_value() {
		if (values.empty())
			throw std::runtime_error("undefined operation");
		return values.back();
	}

	/*! @brief Get the last value
	 * @throws std::runtime_error if the vector is empty
	 */
	const entry_type& last_value() const {
		if (values.empty())
			throw std::runtime_error("undefined operation");
		return values.back();
	}

	/*! @brief Get the value
	 * @throws std::runtime_error if the vector has multiple elements (use values() instead)
	 * @returns the first (and only) value or value_type's default value if the vec is empty
	 */
	const entry_type& value() const {
		static value_type _v;
		if (values.size() > 1)
			throw std::runtime_error("undefined operation");
		else if (values.empty())
			return _v;
		else
			return values.front();
	}

	//! Checks if there are no values
	bool is_empty_value() const {
		return (values.empty());
	}
	/*! @brief Checks if there is only one value
	 * Note: an empty list of values counts as scalar
	 */
	bool is_scalar_value() const {
		return (values.size() <= 1); // strictly speaking, an empty value is a scalar value
	}

private:
	this_type& operator = (const this_type&) = delete;
public:
	this_type& operator = (this_type&& mme) {
		if (this != &mme) {
			base_type::operator = (std::move(mme));
			headers = std::move(mme.headers);
			bound = std::move(mme.bound);
			s_bound = std::move(mme.s_bound);
			values = std::move(mme.values);
		}
		return *this;
	}

	inline bool operator == (const this_type& mme) const { return this->cmp(mme, Cmp_test::eq); }
	inline bool operator != (const this_type& mme) const { return this->cmp(mme, Cmp_test::ne); }
	inline bool operator < (const this_type& mme) const { return this->cmp(mme, Cmp_test::lt); }
	inline bool operator <= (const this_type& mme) const { return this->cmp(mme, Cmp_test::le); }
	inline bool operator >= (const this_type& mme) const { return this->cmp(mme, Cmp_test::ge); }
	inline bool operator > (const this_type& mme) const { return this->cmp(mme, Cmp_test::gt); }

	//! list of headers
	std::map<std::string, std::string> headers;
	//! list of files
	std::vector<entry_type> values;
	//! content-transfer-encoding
	std::string ct_encoding;
	//! error handler (used for propagating errors of POSIX file functions)
	std::function<void(std::basic_string<char_type> const&, std::string const&, int)> error_handler;	
protected:
	
	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return (base_type::am_i_empty() && values.empty() && headers.empty());
	}
	//! cmp operator to generalize comparisons
	int cmp(const this_type& mme, Cmp_test test) const {
		if (test != Cmp_test::ne && this == &mme)
			return true;
		if (!base_type::cmp(mme, test))
			return false;
		if (!::MOSH_FCGI::cmp(headers, mme.headers, test))
			return false;
		// don't check boundary
		return ::MOSH_FCGI::cmp(values, mme.values, test);
	}
		

	/*! @brief Propagate an error.
	 *
	 * Only use this (instead of just throwing) when the error is not the user's fault (e.g. I/O error,
	 * 	allocation error)
	 *
	 * @param efile File name
	 * @param emsg Error string
	 * @param eno Error number (undefined behavior if there's no constant in &lt;cerrno&gt; corresponding
	 * 		to eno
	 * @see _eh_fl_
	 */
	void propagate_error(std::basic_string<char_type>& const efile, 
				std::string const& emsg, int eno)
	{
		(void)(error_handler ? error_handler(efile, emsg, eno) : throw std::runtime_error("Error handler not defined"));
	}
private:
	bool uniqueness_mode;
	std::string bound;
	Boyer_moore_searcher s_bound;

	// enfore noncopy semantics
	MP_mixed_entry(const entry_type&) = delete;
	void add_value(const entry_type&) = delete;
};
} // namespace form
} // namespace http

MOSH_FCGI_END

#endif


