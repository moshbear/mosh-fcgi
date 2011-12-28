//! @file mosh/fcgi/html/element/s.hpp Byte-char elements
/*
 *  Copyright (C) 2011 m0shbear
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */
#ifndef MOSH_FCGI_HTML_ELEMENT__S_HPP
#define MOSH_FCGI_HTML_ELEMENT__S_HPP

#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN
namespace html {
namespace element {

//! Byte-string html elements
namespace s {
	//! @c char specialization of Element&lt;T&gt;
	typedef element::Element<char> Element;
	//! &lt;a&gt;
	const Element a (Type::binary, "a");
	//! &lt;abbr&gt;
	const Element abbr (Type::binary, "abbr");
	//! &lt;address&gt;
	const Element address (Type::binary, "address");
	//! &lt;area&gt;
	const Element area (Type::unary, "area");
	//! &lt;article&gt; (HTML 5)
	const Element article (Type::binary, "article");
	//! &lt;aside&gt; (HTML 5)
	const Element aside (Type::binary, "aside");
	//! &lt;audio&gt; (HTML 5)
	const Element audio (Type::binary, "audio");
	//! &lt;b&gt;
	const Element b (Type::binary, "b");
	//! &lt;base&gt;
	const Element base (Type::unary, "base");
	//! &lt;bdi&gt; (HTML 5)
	const Element bdi (Type::binary, "bdi");
	//! &lt;bdo&gt;
	const Element bdo (Type::binary, "bdo");
	//! &lt;big&gt; (not in HTML 5)
	const Element big (Type::binary, "big");
	//! &lt;blockquote&gt;
	const Element blockquote (Type::binary, "blockquote");
	//! &lt;body&gt;
	const Element body (Type::binary, "body");
	//! &lt;br&gt;
	const Element br (Type::unary, "br");
	//! &lt;button&gt;
	const Element button (Type::binary, "button");
	//! &lt;canvas&gt; (HTML 5)
	const Element canvas (Type::binary, "canvas");
	//! &lt;caption&gt;
	const Element caption (Type::binary, "caption");
	//! &lt;cite&gt;
	const Element cite (Type::binary, "cite");
	//! &lt;code&gt;
	const Element code (Type::binary, "code");
	//! &lt;col&gt;
	const Element col (Type::unary, "col");
	//! &lt;colgroup&gt;
	const Element colgroup (Type::binary, "colgroup");
	//! &lt;command&gt; (HTML 5)
	const Element command (Type::binary, "command");
	//! &lt;datalist&gt; (HTML 5)
	const Element datalist (Type::binary, "datalist");
	//! &lt;dd&gt;
	const Element dd (Type::binary, "dd");
	//! &lt;del&gt;
	const Element del (Type::binary, "del");
	//! &lt;details&gt; (HTML 5)
	const Element details (Type::binary, "details");
	//! &lt;dfn&gt;
	const Element dfn (Type::binary, "dfn");
	//! &lt;div&gt;
	const Element div (Type::binary, "div");
	//! &lt;dl&gt;
	const Element dl (Type::binary, "dl");
	//! &lt;dt&gt;
	const Element dt (Type::binary, "dt");
	//! &lt;em&gt;
	const Element em (Type::binary, "em");
	//! &lt;embed&gt; (HTML 5)
	const Element embed (Type::unary, "embed");
	//! &lt;fieldset&gt;
	const Element fieldset (Type::binary, "fieldset");
	//! &lt;figcaption&gt; (HTML 5)
	const Element figcaption (Type::binary, "figcaption");
	//! &lt;figure&gt; (HTML 5)
	const Element figure (Type::binary, "figure");
	//! &lt;footer&gt; (HTML 5)
	const Element footer (Type::binary, "footer");
	//! &lt;form&gt;
	const Element form (Type::binary, "form");
	//! &lt;frame&gt; (Frameset DTDs only)
	const Element frame (Type::unary, "frame");
	//! &lt;frameset&gt; (Frameset DTDs only)
	const Element frameset (Type::binary, "frameset");
	//! &lt;h1&gt;
	const Element h1 (Type::binary, "h1");
	//! &lt;h2&gt;
	const Element h2 (Type::binary, "h2");
	//! &lt;h3&gt;
	const Element h3 (Type::binary, "h3");
	//! &lt;h4&gt;
	const Element h4 (Type::binary, "h4");
	//! &lt;h5&gt;
	const Element h5 (Type::binary, "h5");
	//! &lt;h6&gt;
	const Element h6 (Type::binary, "h6");
	//! &lt;head&gt;
	const Element head (Type::binary, "head");
	//! &lt;header&gt; (HTML 5)
	const Element header (Type::binary, "header");
	//! &lt;hgroup&gt; (HTML 5)
	const Element hgroup (Type::binary, "hgroup");
	//! &lt;hr&gt;
	const Element hr (Type::unary, "hr");
	//! &lt;html&gt;
	const Element html (Type::binary, "html");
	//! &lt;i&gt;
	const Element i (Type::binary, "i");
	//! &lt;iframe&gt; (Frameset DTDs only / HTML 5)
	const Element iframe (Type::binary, "iframe");
	//! &lt;img&gt;
	const Element img (Type::unary, "img");
	//! &lt;input&gt;
	const Element input (Type::unary, "input");
	//! &lt;ins&gt;
	const Element ins (Type::binary, "ins");
	//! &lt;keygen&gt; (HTML 5)
	const Element keygen (Type::unary, "keygen");
	//! &lt;kbd&gt;
	const Element kbd (Type::binary, "kbd");
	//! &lt;label&gt;
	const Element label (Type::binary, "label");
	//! &lt;legend&gt;
	const Element legend (Type::binary, "legend");
	//! &lt;li&gt;
	const Element li (Type::binary, "li");
	//! &lt;link&gt;
	const Element link (Type::unary, "link");
	//! &lt;mark&gt; (HTML 5)
	const Element mark (Type::binary, "mark");
	//! &lt;menu&gt;
	const Element menu (Type::binary, "menu");
	//! &lt;meta&gt;
	const Element meta (Type::unary, "meta");
	//! &lt;meter&gt; (HTML 5)
	const Element meter (Type::binary, "meter");
	//! &lt;nav&gt; (HTML 5)
	const Element nav (Type::binary, "nav");
	//! &lt;noframes&gt; (Frameset DTDs only)
	const Element noframes (Type::binary, "noframes");
	//! &lt;noscript&gt;
	const Element noscript (Type::binary, "noscript");
	//! &lt;object&gt;
	const Element object (Type::binary, "object");
	//! &lt;ol&gt;
	const Element ol (Type::binary, "ol");
	//! &lt;optgroup&gt;
	const Element optgroup (Type::binary, "optgroup");
	//! &lt;option&gt;
	const Element option (Type::binary, "option");
	//! &lt;output&gt; (HTML 5)
	const Element output (Type::binary, "output");
	//! &lt;p&gt;
	const Element p (Type::binary, "p");
	//! &lt;param&gt;
	const Element param (Type::unary, "param");
	//! &lt;pre&gt;
	const Element pre (Type::binary, "pre");
	//! &lt;progress&gt; (HTML 5)
	const Element progress (Type::binary, "progress");
	//! &lt;q&gt;
	const Element q (Type::binary, "q");
	//! &lt;rb&gt; (XHTML 1.1 Ruby)
	const Element rb (Type::binary, "rb");
	//! &lt;rb&gt; (XHTML 1.1 Ruby)
	const Element rbc (Type::binary, "rbc");
	//! &lt;rp&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element rp (Type::binary, "rp");
	//! &lt;rt&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element rt (Type::binary, "rt");
	//! &lt;rtc&gt; (XHTML 1.1 Ruby)
	const Element rtc (Type::binary, "rtc");
	//! &lt;ruby&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element ruby (Type::binary, "ruby");
	//! &lt;s&gt;
	const Element s (Type::binary, "s");
	//! &lt;samp&gt;
	const Element samp (Type::binary, "samp");
	//! &lt;script&gt;
	const Element script (Type::binary, "script");
	//! &lt;section&gt; (HTML 5)
	const Element section (Type::binary, "section"); 
	//! &lt;select&gt;
	const Element select (Type::binary, "select");
	//! &lt;small&gt;
	const Element small (Type::binary, "small");
	//! &lt;source&gt; (HTML 5)
	const Element source (Type::binary, "source"); 
	//! &lt;span&gt;
	const Element span (Type::binary, "span");
	//! &lt;strong&gt;
	const Element strong (Type::binary, "strong");
	//! &lt;style&gt;
	const Element style (Type::binary, "style");
	//! &lt;sub&gt;
	const Element sub (Type::binary, "sub");
	//! &lt;summary&gt; (HTML 5)
	const Element summary (Type::binary, "summary"); 
	//! &lt;sup&gt;
	const Element sup (Type::binary, "sup");
	//! &lt;table&gt;
	const Element table (Type::binary, "table");
	//! &lt;tbody&gt;
	const Element tbody (Type::binary, "tbody");
	//! &lt;td&gt;
	const Element td (Type::binary, "td");
	//! &lt;textarea&gt;
	const Element textarea (Type::binary, "textarea");
	//! &lt;tfoot&gt;
	const Element tfoot (Type::binary, "tfoot");
	//! &lt;th&gt;
	const Element th (Type::binary, "th");
	//! &lt;thead&gt;
	const Element thead (Type::binary, "thread");
	//! &lt;time&gt; (HTML 5)
	const Element time (Type::binary, "time");
	//! &lt;title&gt;
	const Element title (Type::binary, "title");
	//! &lt;tr&gt;
	const Element tr (Type::binary, "tr");
	//! &lt;track&gt; (HTML 5)
	const Element track (Type::unary, "track");
	//! &lt;tt&gt; (not in HTML 5)
	const Element tt (Type::binary, "tt");
	//! &lt;ul&gt; 
	const Element ul (Type::binary, "ul");
	//! &lt;var&gt;
	const Element var (Type::binary, "var");	
	//! &lt;video&gt; (HTML 5)
	const Element video (Type::binary, "video");
	//!  &lt;wbr&gt; (HTML 5)
	const Element wbr (Type::binary, "wbr"); 
	
	//! &lt;!-- ... --&gt;
	const Element comment (Type::comment, "!--");
	
	//! @c char specialization of Html_begin&lt;T&gt;
	typedef element::HTML_begin<char> HTML_begin;
	//! @c char specialization of Html_end&lt;T&gt;
	typedef element::HTML_end<char> HTML_end;
	//! @c char specialization of Body_begin&lt;T&gt;
	typedef element::Body_begin<char> Body_begin;
	//! @c char specialization of Body_end&lt;T&gt;
	typedef element::Body_end<char> Body_end;
	//! &lt;html&gt;
	const HTML_begin html_begin;
	//! &lt;/html&gt;
	const HTML_end html_end;
	//! &lt;body&gt;
	const Body_begin body_begin;
	//! &lt;/body&gt;
	const Body_end body_end;

	//@{
	//! @c char specialization of P
	std::pair<std::string, std::string>
	P(const std::string& s1, const std::string& s2) {
		return element::P(s1, s2);
	}
	//! @c char specialization of P
	std::pair<std::string, std::string>
	P(std::string&& s1, std::string&& s2) {
		return element::P(std::move(s1), std::move(s2));
	}
	//@}
}

}
}

MOSH_FCGI_END

#endif
