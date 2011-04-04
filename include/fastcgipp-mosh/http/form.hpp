//! @file http/form.hpp - HTTP Session form data
/***************************************************************************
* Copyright (C) 2011 Andrey Vul                                            *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#ifndef FASTCGI_HTTP_FORM
#define FASTCGI_HTTP_FORM

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <fastcgipp-mosh/bits/cmp.hpp>
#include <fastcgipp-mosh/bits/boyer_moore.hpp>
#ifdef FASTCGIPP_USE_HYBRID_VECTOR
#define VEC_OPS__HV
#include <hybrid_vector>
#endif
#ifdef FASTCGIPP_USE_STXXL
#define VEC_OPS__STXXL
#include <stxxl/vector>
#endif
#define VEC_OPS__BEGIN namespace VecOps {
#define VEC_OPS__END }
#include <fastcgipp-mosh/bits/vec_ops.hpp>
#undef VEC_OPS__BEGIN
#undef VEC_OPS__END

namespace Fastcgipp_m0sh
{
namespace Http
{
/*
 * List of classes:
 *
 * Class_name       | multiple values | headers | files
 * -----------------------------------------------------
 *   Entry          |     yes         |    no   |   no
 *   MP_entry       |     no          |    yes  |   yes
 *   MP_mixed_entry |     yes         |    yes  |   yes
 *
 *   The rationale for declaring certain fields public is as follows:
 *   	There are no invariants for which a particular value is a dependency,
 *   	so the overhead of getters and setters is unwarranted. Furthermore,
 *   	there are no unintented side effects from particular values, so it
 *   	is safe to allow blind access to them outside the class/subclass.
 *
 */
namespace Form
{

/** @brief Type of object of subclass.
 *
 * Data<T> uses the value of this object to compare if the derived parts
 * are comparable.
 */
enum Type {
	//! K=V form entry
	t_form_entry,
	//! form/multipart-data entry
	t_mp_entry,
	//! form/multipart-mixed entry
	t_mp_mixed_entry
};

/** @brief Base class for form objects.
 *
 * This class stores the name of the form element and the type of the form
 * element (@see Type).
 *
 * @remark Comparison operators throw std::runtime_error if the types are unequal
 *
 * @tparam char_type character type
 */
template <class char_type>
class Data {
private:
	typedef Data<char_type> this_type;
public:
	/** @brief Create a new form data key of a given type.
	 * @param f type of object
	 */
	Data(Type f) : type_(f)	{ }
	/** @brief Create a new form data key of a given type, with a given name.
	 * @param f type of object
	 * @param[in] _name name of form element
	 */
	Data(Type f, const std::basic_string<char_type>& _name)
	: type_(f), name_(_name)
	{ }
	/** @brief Create a new form data key by copying the existing one.
	 * Atomic counter is unchanged.
	 * Type must still be specified.
	 * @param f type of object
	 * @param[in] entry_ entry to copy values from
	 */
	Data(Type f, const this_type& entry_)
	: type_(f), name_(entry_.name_)
	{ }

	virtual ~Data()
	{ }
	
	this_type& operator = (const this_type& entry_) {
		if (type_ != entry_.type_)
			throw std::runtime_error("caught attempt to use base type of incompatible derived types");
		if (this != &entry_) {
			name_ = entry_.name_;
		}
		return *this;
	}

	inline bool operator == (const this_type& entry_) const { return (cmp(entry_) != 0); }
	inline bool operator != (const this_type& entry_) const { return (cmp(entry_) == 0); }
	inline bool operator < (const this_type& entry_) const { return (cmp(entry_) < 0); }
	inline bool operator <= (const this_type& entry_) const { return (cmp(entry_) <= 0); }
	inline bool operator >= (const this_type& entry_) const { return (cmp(entry_) >= 0); }
	inline bool operator > (const this_type& entry_) const { return (cmp(entry_) > 0); }

	//! entry name
	std::basic_string<char_type> name_;
protected:
	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return name_.empty();
	}
	//! cmp operator to generalize comparisons
	int cmp(const this_type& entry_) const {
		if (type_ != entry_.type_)
			throw std::runtime_error("caught attempt to compare base type of incompatible derived types");
		return cmp(name_, entry_.name_);
	}
private:
	Type type_;
};

/** @brief Form entry
 * The usual form entry, this class is appropriate for holding GETs and x-www-formurl-encoded
 * vars. To allow for duplicate values, an internal vector is employed.
 *
 * @tparam char_type character type
 * @tparam value_type value type
 */
template <class char_type, class value_type = std::basic_string<char_type> >
class Entry : public Data<char_type> {
private:
	typedef Entry<char_type, value_type> this_type;
	typedef Data<char_type> base_type;
public:
	Entry()	: base_type(t_form_entry) { }
	/** @brief Create a form entry with a given name and value
	 * @param[in] _name entry name
	 * @param[in] value_ initial value
	 */
	Entry(const std::basic_string<char_type>& _name, const value_type& value_ = value_type())
	: base_type(t_form_entry, _name)
	{
		add_value(value_);
	}
	/** @brief Create a form entry which is a duplicate of the input
	 * @param[in] entry_ input
	 */
	Entry(const this_type& entry_)
	: base_type(entry_), values(entry_.values)
	{ }

	virtual ~Entry()
	{ }

	/** @brief Add a value to the values list
	 * This adds a value to the values list.
	 * If uniqueness is enabled, std::find checks for duplicates, making insertion
	 * O(n). Otherwise, it is O(1).
	 * @param[in] value_ the value to add
	 */
	void add_value(const value_type& value_) {
		if ((!uniqueness_mode)
		|| (std::find(values.begin(), values.end(), value_) == values.end()))
			values.push_back(value_);
	}
	
	//! Enables uniqueness mode
	void enable_unique_mode() {
		uniqueness_mode = 1;
	}
	//! Disables uniqueness mode
	void disable_unique_mode() {
		uniqueness_mode = 0;
	}
	
	/** @brief Get a ref to the last value
	 * @throws std::runtime_error if the vector is empty
	 */
	value_type& last_value() {
		if (values.empty())
			throw std::runtime_error("undefined operation");
		return values.back();
	}

	/** @brief Get the value
	 * @throws std::runtime_error if the vector has multiple elements (use values() instead)
	 * @returns the first (and only) value or value_type's default value if the vec is empty
	 */
	const value_type& value() const {
		if (values.size() > 1)
			throw std::runtime_error("undefined operation");
		else if (values.empty())
			return this->default_value_;
		else
			return values.front();
	}

	//! Checks if there are no values
	bool is_empty_value() const {
		return (values.empty());
	}
	/** @brief Checks if there is only one value
	 * Note: an empty list of values counts as scalar
	 */
	bool is_scalar_value() const {
		return (values.size() <= 1); // strictly speaking, an empty value _is_ a scalar value
	}

	this_type& operator = (const this_type& entry_) {
		if (this != &entry_) {
			base_type::operator = (entry_);
			values = entry_.values;
		}
		return *this;
	}

	/** @brief Left-shift operator
	 * This is used to shift in an additional value to the end of the vec.
	 * @param[in] val the value to add
	 * @returns *this
	 */
	inline this_type& operator << (const value_type& val) {
		add_value(val);
		return *this;
	}			
	

	inline bool operator == (const this_type& entry_) const { return (cmp(entry_) != 0); }
	inline bool operator != (const this_type& entry_) const { return (cmp(entry_) == 0); }
	inline bool operator < (const this_type& entry_) const { return (cmp(entry_) < 0); }
	inline bool operator <= (const this_type& entry_) const { return (cmp(entry_) <= 0); }
	inline bool operator >= (const this_type& entry_) const { return (cmp(entry_) >= 0); }
	inline bool operator > (const this_type& entry_) const { return (cmp(entry_) > 0); }
	
	//! \brief List of values for this particular entry
	std::vector<value_type> values;
protected:
	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return (base_type::am_i_empty() && values.empty());
	}
	//! cmp operator to generalize comparisons
	int cmp(const this_type& entry_) const {
		int pcmp = base_type::cmp(entry_);
		if (pcmp != 0) return pcmp;
		return cmp(values, entry_.values);
	}
private:
	bool uniqueness_mode;
	static value_type default_value_;
};

template <class char_type, class buffer_type = std::vector<char> >
class MP_entry : public Data<char_type> {
private:
	typedef MP_entry<char_type, buffer_type> this_type;
	typedef typename buffer_type::const_iterator::size_type size_type;
	typedef Data<char_type> base_type;
public:
	MP_entry() : base_type(t_mp_entry) { }
	
	/** @brief Create a new MP_entry
	 * @param[in] _name entry name
	 * @param[in] filename entry filename
	 * @param[in] content_type content type
	 * @param[in] data_begin start of file data
	 * @param[in] data_end end of file data
	 */
	template <typename InputIterator>
	MP_entry(const std::basic_string<char_type>& _name, const std::basic_string<char_type>& filename_,
		const std::basic_string<char_type> content_type_, InputIterator data_begin,
		InputIterator data_end)
	: base_type(t_mp_entry, _name), filename(filename_), content_type(content_type_), data_(new buffer_type())
	{
		VecOps::vec_ops<buffer_type>::assign(*data_, data_begin, data_end);
	}
	MP_entry(const this_type& mpe)
	: base_type(mpe), filename(mpe.filename), content_type(mpe.content_type),
		data_(mpe.data_), headers(mpe.headers), is_copy(true)
	{ }

	virtual ~MP_entry()
	{ }

	/** @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 */
	void add_header(const std::string& k, const std::basic_string<char_type>& v) {
		headers[k] = v;
	}

	/** @brief Add data to the file
	 * Copy-on-write is implemented here.
	 * @param[in] s start of file data
	 * @param[in] e end of file data
	 */
	template <typename InputIterator>
	void append_data(InputIterator s, InputIterator e) {
		if (is_copy) {
			buffer_type* old_data = data_.get();
			data_.reset(new buffer_type());
			VecOps::vec_ops<buffer_type>::append(*data_, old_data->begin(), old_data->end());
			is_copy = false;
		}	
		VecOps::vec_ops<buffer_type>::append(*data_, s, e);
	}

	//! Get a cref to the data
	const buffer_type& data() const { return *data_; }

	//! Get the file size
	size_type file_size() const {
		return data_->size();
	}
	
	//! Check if the file is empty
	bool is_empty_file() const {
		return !(data_->size());
	}

	this_type& operator = (const this_type& mpe) {
		if (this != &mpe) {
			base_type::operator = (mpe);
			filename = mpe.filename;
			content_type = mpe.content_type;
			headers = mpe.headers;
			data_ = mpe.data_;
			is_copy = true;
		}
		return *this;
	}

	/** @brief Left-shift operator
	 * This is used to shift in data to the end of the file.
	 * @remark Iterator pair overload
	 * @param it iterator pair of buffer to add ([.first, .second))
	 * @returns *this
	 */
	template <typename InputIterator>
	this_type& operator << (std::pair<InputIterator, InputIterator> it) {
		append_data(it.first, it.second);
		return *this;
	}
	/** @brief Left-shift operator
	 * This is used to shift in data to the end of the file.
	 * @remark std::vector<char> overload
	 * @param[in] v input buffer
	 * @returns *this
	 */
	this_type& operator << (const std::vector<char>& v) {
		append_data(v.begin(), v.end());
		return *this;
	}
	/** @brief Left-shift operator
	 * This is used to shift in data to the end of the file.
	 * @remark std::string overload
	 * @param[in] s input buffer
	 * @returns *this
	 */
	this_type& operator << (const std::string& s) {
		append_data(s.begin(), s.end());
		return *this;
	}
	
	inline bool operator == (const this_type& mpe) const { return (cmp(mpe) != 0); }
	inline bool operator != (const this_type& mpe) const { return (cmp(mpe) == 0); }
	inline bool operator < (const this_type& mpe) const { return (cmp(mpe) < 0); }
	inline bool operator <= (const this_type& mpe) const { return (cmp(mpe) <= 0); }
	inline bool operator >= (const this_type& mpe) const { return (cmp(mpe) >= 0); }
	inline bool operator > (const this_type& mpe) const { return (cmp(mpe) > 0); }

	//! the file name; if empty, then data is form input instead of file input
	std::basic_string<char_type> filename;
	//! value of Content-Type header
	std::basic_string<char_type> content_type;
	//! list of all headers
	std::map<std::string, std::basic_string<char_type> > headers;
protected:
	bool am_i_empty() const {
		return (base_type::am_i_empty()
		&& filename.empty()
		&& content_type.empty()
		&& headers.empty()
		&& is_empty_file());
	}

	//! cmp operator to generalize comparisons
	int cmp(const this_type& mpe) const {
		int pcmp = base_type::operator==(mpe);
		if (pcmp != 0) return pcmp;
		if ((pcmp = cmp(!am_i_empty(), !mpe.am_i_empty())) != 0)
			return pcmp;
		if ((pcmp = cmp(filename, mpe.filename)) != 0)
			return pcmp;
		if ((pcmp = cmp(content_type, mpe.content_type)) != 0)
			return pcmp;
		if ((pcmp = cmp(headers, mpe.headers)) != 0)
			return pcmp;
		// Compare file size
		if ((pcmp = cmp(data_->size(), data_->size())) != 0)
			return pcmp;
		if (is_empty_file())
			return 0;
		// Compare filedata objects
		if ((pcmp = cmp(data_.get(), mpe.data_.get())) != 0)
			return pcmp;
		return cmp(*data_, *mpe.data_);
	}
private:
	boost::shared_ptr<buffer_type> data_;
	bool is_copy; // true if data_ points to copy; this lets append_data perform COW
};

/** @brief Multipart/mixed entry
 * This class holds (a) multipart/mixed entry(/ies), which is a bunch of headers followed
 * by multipart/form-data fields
 * @tparam char_type type of char to use in strings
 * @tparam buffer_type type of buffer to use to hold file data. If the type
 * 			doesn't exist in bits/vec_ops.hpp, make a VEC_OPS__SPEC
 * 			for the new vector type.
 * 			Currently available options are std::vector, stxxl::vector,
 * 			and hybid_vector.
 */
template <class char_type, class buffer_type = std::vector<char> >
class MP_mixed_entry : public Data<char_type> {
private:
	typedef MP_mixed_entry<char_type, buffer_type> this_type;
	typedef MP_entry<char_type, buffer_type> file_type;
	typedef Data<char_type> base_type;
public:
	MP_mixed_entry() : base_type(t_mp_mixed_entry) { }
	
	/** @brief Create a new MP_mixed_entry
	 * @param[in] name entry name
	 * @param[in] _boundary boundary string
	 */
	MP_mixed_entry(const std::basic_string<char_type>& _name, const std::string& _boundary)
	: base_type(t_mp_mixed_entry, _name), boundary_(_boundary)
	{
		s_bound = Boyer_moore_searcher(_boundary);
	}
	
	MP_mixed_entry(const this_type& entry_)
	: base_type(entry_), values(entry_.values), boundary_(entry_.boundary_), s_bound(entry_.s_bound)
	{ }
	
	//! @Brief Create a new MP_mixed_entry from an existing MP_entry
	MP_mixed_entry(const file_type& mpe)
	: base_type(t_mp_mixed_entry, mpe.name), headers(mpe.headers) 
	{ }

	virtual ~MP_mixed_entry()
	{ }

	/** @brief Add a header
	 * @param[in] k key
	 * @param[in] v value
	 */
	void add_header(const std::string& k, const std::basic_string<char_type>& v) {
		headers[k] = v;
	}

	/** @brief Add a value to the values list
	 * This adds a value to the values list. Uniqueness check is determined by unique checking
	 * mode.
	 * @param[in] value_ the value to add
	 */
	void add_value(const file_type& value_) {
		if ((!uniqueness_mode)
		|| (std::find(values.begin(), values.end(), value_) == values.end()))
			values.push_back(value_);
	}
	
	/** @brief sets the boundary
	 * Sets the boundary string and (re-)initializes the searcher
	 * @param[in] s the new boundary string
	 * @returns *this
	 */
	this_type& set_boundary(const std::string& s) {
		boundary_ = s;
		s_bound = Boyer_moore_searcher(s);
		return *this;
	}
	
	const std::string& boundary() const {
		return boundary_;
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
	
	/** @brief Get a ref to the last value
	 * @throws std::runtime_error if the vector is empty
	 */
	file_type& last_value() {
		if (values.empty())
			throw std::runtime_error("undefined operation");
		return values.back();
	}
	
	/** @brief Get the value
	 * @throws std::runtime_error if the vector has multiple elements (use values() instead)
	 * @returns the first (and only) value or value_type's default value if the vec is empty
	 */
	const file_type& value() const {
		if (values.size() > 1)
			throw std::runtime_error("undefined operation");
		else if (values.empty())
			return this->empty_string;
		else
			return values.front();
	}

	//! Checks if there are no values
	bool is_empty_value() const {
		return (values.empty());
	}
	/** @brief Checks if there is only one value
	 * Note: an empty list of values counts as scalar
	 */
	bool is_scalar_value() const {
		return (values.size() <= 1); // strictly speaking, an empty value _is_ a scalar value
	}

	this_type& operator = (const this_type& entry_) {
		if (this != &entry_) {
		
			base_type::operator = (entry_);
			headers = entry_.headers;
			boundary = entry_.boundary;
			s_bound = entry_.s_bound;
			value = entry_.value;
		}
		return *this;
	}

	this_type& operator << (const file_type& val) {
		add_value(val);
		return *this;
	}			
	
	inline bool operator == (const this_type& mme) const { return (cmp(mme) != 0); }
	inline bool operator != (const this_type& mme) const { return (cmp(mme) == 0); }
	inline bool operator < (const this_type& mme) const { return (cmp(mme) < 0); }
	inline bool operator <= (const this_type& mme) const { return (cmp(mme) <= 0); }
	inline bool operator >= (const this_type& mme) const { return (cmp(mme) >= 0); }
	inline bool operator > (const this_type& mme) const { return (cmp(mme) > 0); }

	//! list of headers
	std::map<std::string, std::basic_string<char_type> > headers;
	//! list of files
	std::vector<file_type> values;
protected:
	
	//! Returns true if this form entry is indeed empty (i.e. name == "")
	bool am_i_empty() const {
		return (base_type::am_i_empty() && values.empty() && headers.empty());
	}
	//! cmp operator to generalize comparisons
	int cmp(const this_type& mme) const {
		int pcmp = base_type::cmp(mme);
		if (pcmp != 0) return pcmp;
		if ((pcmp = cmp(headers, mme.headers)) != 0) return pcmp;
		// boundary is unchecked because it's irrelevant to equivalence
		return cmp(values, mme.values);
	}
		

private:
	bool uniqueness_mode;
	std::string boundary_;
	Boyer_moore_searcher s_bound;
};
} // namespace Form
} // namespace Http
} // namespace Fastcgipp_m0sh

#endif


