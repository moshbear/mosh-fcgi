//! @file  mosh/fcgi/html/element/ws.hpp Wide-char elements
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

#ifndef MOSH_FCGI_HTML_ELEMENT__WS_HPP
#define MOSH_FCGI_HTML_ELEMENT__WS_HPP

#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN
namespace html {
namespace element {

//! Wide-string html elements
namespace ws {
	//! @c wchar_t specialization of Element&lt;T&gt;
	typedef element::Element<wchar_t> Element;
	//! &lt;a&gt;
	const Element a (Type::binary, L"a");
	//! &lt;abbr&gt;
	const Element abbr (Type::binary, L"abbr");
	//! &lt;address&gt;
	const Element address (Type::binary, L"address");
	//! &lt;area&gt;
	const Element area (Type::unary, L"area");
	//! &lt;article&gt; (HTML 5)
	const Element article (Type::binary, L"article");
	//! &lt;aside&gt; (HTML 5)
	const Element aside (Type::binary, L"aside");
	//! &lt;audio&gt; (HTML 5)
	const Element audio (Type::binary, L"audio");
	//! &lt;b&gt;
	const Element b (Type::binary, L"b");
	//! &lt;base&gt;
	const Element base (Type::unary, L"base");
	//! &lt;bdi&gt; (HTML 5)
	const Element bdi (Type::binary, L"bdi");
	//! &lt;bdo&gt;
	const Element bdo (Type::binary, L"bdo");
	//! &lt;big&gt; (not in HTML 5)
	const Element big (Type::binary, L"big");
	//! &lt;blockquote&gt;
	const Element blockquote (Type::binary, L"blockquote");
	//! &lt;body&gt;
	const Element body (Type::binary, L"body");
	//! &lt;br&gt;
	const Element br (Type::unary, L"br");
	//! &lt;button&gt;
	const Element button (Type::binary, L"button");
	//! &lt;canvas&gt; (HTML 5)
	const Element canvas (Type::binary, L"canvas");
	//! &lt;caption&gt;
	const Element caption (Type::binary, L"caption");
	//! &lt;cite&gt;
	const Element cite (Type::binary, L"cite");
	//! &lt;code&gt;
	const Element code (Type::binary, L"code");
	//! &lt;col&gt;
	const Element col (Type::unary, L"col");
	//! &lt;colgroup&gt;
	const Element colgroup (Type::binary, L"colgroup");
	//! &lt;command&gt; (HTML 5)
	const Element command (Type::binary, L"command");
	//! &lt;datalist&gt; (HTML 5)
	const Element datalist (Type::binary, L"datalist");
	//! &lt;dd&gt;
	const Element dd (Type::binary, L"dd");
	//! &lt;del&gt;
	const Element del (Type::binary, L"del");
	//! &lt;details&gt; (HTML 5)
	const Element details (Type::binary, L"details");
	//! &lt;dfn&gt;
	const Element dfn (Type::binary, L"dfn");
	//! &lt;div&gt;
	const Element div (Type::binary, L"div");
	//! &lt;dl&gt;
	const Element dl (Type::binary, L"dl");
	//! &lt;dt&gt;
	const Element dt (Type::binary, L"dt");
	//! &lt;em&gt;
	const Element em (Type::binary, L"em");
	//! &lt;embed&gt; (HTML 5)
	const Element embed (Type::unary, L"embed");
	//! &lt;fieldset&gt;
	const Element fieldset (Type::binary, L"fieldset");
	//! &lt;figcaption&gt; (HTML 5)
	const Element figcaption (Type::binary, L"figcaption");
	//! &lt;figure&gt; (HTML 5)
	const Element figure (Type::binary, L"figure");
	//! &lt;footer&gt; (HTML 5)
	const Element footer (Type::binary, L"footer");
	//! &lt;form&gt;
	const Element form (Type::binary, L"form");
	//! &lt;frame&gt; (Frameset DTDs only)
	const Element frame (Type::unary, L"frame");
	//! &lt;frameset&gt; (Frameset DTDs only)
	const Element frameset (Type::binary, L"frameset");
	//! &lt;h1&gt;
	const Element h1 (Type::binary, L"h1");
	//! &lt;h2&gt;
	const Element h2 (Type::binary, L"h2");
	//! &lt;h3&gt;
	const Element h3 (Type::binary, L"h3");
	//! &lt;h4&gt;
	const Element h4 (Type::binary, L"h4");
	//! &lt;h5&gt;
	const Element h5 (Type::binary, L"h5");
	//! &lt;h6&gt;
	const Element h6 (Type::binary, L"h6");
	//! &lt;head&gt;
	const Element head (Type::binary, L"head");
	//! &lt;header&gt; (HTML 5)
	const Element header (Type::binary, L"header");
	//! &lt;hgroup&gt; (HTML 5)
	const Element hgroup (Type::binary, L"hgroup");
	//! &lt;hr&gt;
	const Element hr (Type::unary, L"hr");
	//! &lt;html&gt;
	const Element html (Type::binary, L"html");
	//! &lt;i&gt;
	const Element i (Type::binary, L"i");
	//! &lt;iframe&gt; (Frameset DTDs only / HTML 5)
	const Element iframe (Type::binary, L"iframe");
	//! &lt;img&gt;
	const Element img (Type::unary, L"img");
	//! &lt;input&gt;
	const Element input (Type::unary, L"input");
	//! &lt;ins&gt;
	const Element ins (Type::binary, L"ins");
	//! &lt;keygen&gt; (HTML 5)
	const Element keygen (Type::unary, L"keygen");
	//! &lt;kbd&gt;
	const Element kbd (Type::binary, L"kbd");
	//! &lt;label&gt;
	const Element label (Type::binary, L"label");
	//! &lt;legend&gt;
	const Element legend (Type::binary, L"legend");
	//! &lt;li&gt;
	const Element li (Type::binary, L"li");
	//! &lt;link&gt;
	const Element link (Type::unary, L"link");
	//! &lt;mark&gt; (HTML 5)
	const Element mark (Type::binary, L"mark");
	//! &lt;menu&gt;
	const Element menu (Type::binary, L"menu");
	//! &lt;meta&gt;
	const Element meta (Type::unary, L"meta");
	//! &lt;meter&gt; (HTML 5)
	const Element meter (Type::binary, L"meter");
	//! &lt;nav&gt; (HTML 5)
	const Element nav (Type::binary, L"nav");
	//! &lt;noframes&gt; (Frameset DTDs only)
	const Element noframes (Type::binary, L"noframes");
	//! &lt;noscript&gt;
	const Element noscript (Type::binary, L"noscript");
	//! &lt;object&gt;
	const Element object (Type::binary, L"object");
	//! &lt;ol&gt;
	const Element ol (Type::binary, L"ol");
	//! &lt;optgroup&gt;
	const Element optgroup (Type::binary, L"optgroup");
	//! &lt;option&gt;
	const Element option (Type::binary, L"option");
	//! &lt;output&gt; (HTML 5)
	const Element output (Type::binary, L"output");
	//! &lt;p&gt;
	const Element p (Type::binary, L"p");
	//! &lt;param&gt;
	const Element param (Type::unary, L"param");
	//! &lt;pre&gt;
	const Element pre (Type::binary, L"pre");
	//! &lt;progress&gt; (HTML 5)
	const Element progress (Type::binary, L"progress");
	//! &lt;q&gt;
	const Element q (Type::binary, L"q");
	//! &lt;rb&gt; (XHTML 1.1 Ruby)
	const Element rb (Type::binary, L"rb");
	//! &lt;rb&gt; (XHTML 1.1 Ruby)
	const Element rbc (Type::binary, L"rbc");
	//! &lt;rp&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element rp (Type::binary, L"rp");
	//! &lt;rt&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element rt (Type::binary, L"rt");
	//! &lt;rtc&gt; (XHTML 1.1 Ruby)
	const Element rtc (Type::binary, L"rtc");
	//! &lt;ruby&gt; (XHTML 1.1 / HTML 5 Ruby)
	const Element ruby (Type::binary, L"ruby");
	//! &lt;s&gt;
	const Element s  (Type::binary, L"s");
	//! &lt;samp&gt;
	const Element samp (Type::binary, L"samp");
	//! &lt;script&gt;
	const Element script (Type::binary, L"script");
	//! &lt;section&gt; (HTML 5)
	const Element section (Type::binary, L"section"); 
	//! &lt;select&gt;
	const Element select (Type::binary, L"select");
	//! &lt;small&gt;
	const Element small (Type::binary, L"small");
	//! &lt;source&gt; (HTML 5)
	const Element source (Type::binary, L"source"); 
	//! &lt;span&gt;
	const Element span (Type::binary, L"span");
	//! &lt;strong&gt;
	const Element strong (Type::binary, L"strong");
	//! &lt;style&gt;
	const Element style (Type::binary, L"style");
	//! &lt;sub&gt;
	const Element sub (Type::binary, L"sub");
	//! &lt;summary&gt; (HTML 5)
	const Element summary (Type::binary, L"summary"); 
	//! &lt;sup&gt;
	const Element sup (Type::binary, L"sup");
	//! &lt;table&gt;
	const Element table (Type::binary, L"table");
	//! &lt;tbody&gt;
	const Element tbody (Type::binary, L"tbody");
	//! &lt;td&gt;
	const Element td (Type::binary, L"td");
	//! &lt;textarea&gt;
	const Element textarea (Type::binary, L"textarea");
	//! &lt;tfoot&gt;
	const Element tfoot (Type::binary, L"tfoot");
	//! &lt;th&gt;
	const Element th (Type::binary, L"th");
	//! &lt;thead&gt;
	const Element thead (Type::binary, L"thread");
	//! &lt;time&gt; (HTML 5)
	const Element time (Type::binary, L"time");
	//! &lt;title&gt;
	const Element title (Type::binary, L"title");
	//! &lt;tr&gt;
	const Element tr (Type::binary, L"tr");
	//! &lt;track&gt; (HTML 5)
	const Element track (Type::unary, L"track");
	//! &lt;tt&gt; (not in HTML 5)
	const Element tt (Type::binary, L"tt");
	//! &lt;ul&gt; 
	const Element ul (Type::binary, L"ul");
	//! &lt;var&gt;
	const Element var (Type::binary, L"var");	
	//! &lt;video&gt; (HTML 5)
	const Element video (Type::binary, L"video");
	//!  &lt;wbr&gt; (HTML 5)
	const Element wbr (Type::binary, L"wbr"); 
	
	//! &lt;!-- ... --&gt;
	const Element comment (Type::comment, L"!--");
	
	//! @c wchar_t specialization of Html_begin&lt;T&gt;
	typedef element::HTML_begin<wchar_t> HTML_begin;
	//! @c wchar_t specialization of Html_end&lt;T&gt;
	typedef element::HTML_end<wchar_t> HTML_end;
	//! @c wchar_t specialization of Body_begin&lt;T&gt;
	typedef element::Body_begin<wchar_t> Body_begin;
	//! @c wchar_t specialization of Body_end&lt;T&gt;
	typedef element::Body_end<wchar_t> Body_end;
	//! &lt;html&gt;
	const HTML_begin html_begin;
	//! &lt;/html&gt;
	const HTML_end html_end;
	//! &lt;body&gt;
	const Body_begin body_begin;
	//! &lt;/body&gt;
	const Body_end body_end;

	//@{
	//! @c wchar_t specialization of P
	std::pair<std::wstring, std::wstring>
	P(const std::wstring& s1, const std::wstring& s2) {
		return element::P(s1, s2);
	}
	//! @c wchar_t specialization of P
	std::pair<std::wstring, std::wstring>
	P(std::wstring&& s1, std::wstring&& s2) {
		return element::P(std::move(s1), std::move(s2));
	}
	//@}
	//@{
	//! @c wchar_t specialization of repeat
	std::wstring repeat(Element const& e,
			   std::initializer_list<Element::string> vals)
	{
		return element::repeat(e, vals);
	}
	//! @c wchar_t specialization of repeat
	std::wstring repeat(Element const& e,
			   Element::attribute const& attr,
			   std::initializer_list<Element::string> vals)
	{
		return element::repeat(e, attr, vals);
	}
	//! @c wchar_t specialization of repeat
	std::wstring repeat(Element const& e,
			   std::initializer_list<Element::attribute> attrs,
			   std::initializer_list<Element::string> vals)
	{
		return element::repeat(e, attrs, vals);	
	}
	//@}
	
}

}
}

MOSH_FCGI_END

#endif
