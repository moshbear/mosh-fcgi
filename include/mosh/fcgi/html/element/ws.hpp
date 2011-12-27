//! @file mosh/fcgi/html/element/ws.hpp Wide-char elements
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
	//! @c wchar_t specialization of Element<T>
	typedef element::Element<wchar_t> Element;
	//! <a>
	const Element a (Type::binary, "a");
	//! <abbr>
	const Element abbr (Type::binary, "abbr");
	//! <address>
	const Element address (Type::binary, "address");
	//! <area>
	const Element area (Type::unary, "area");
	//! <article> (HTML 5)
	const Element article (Type::binary, "article");
	//! <aside> (HTML 5)
	const Element aside (Type::binary, "aside");
	//! <audio> (HTML 5)
	const Element audio (Type::binary, "audio");
	//! <b>
	const Element b (Type::binary, "b");
	//! <base>
	const Element base (Type::unary, "base");
	//! <bdi> (HTML 5)
	const Element bdi (Type::binary, "bdi");
	//! <bdo>
	const Element bdo (Type::binary, "bdo");
	//! <big> (not in HTML 5)
	const Element big (Type::binary, "big");
	//! <blockquote>
	const Element blockquote (Type::binary, "blockquote");
	//! <body>
	const Element body (Type::binary, "body");
	//! <br>
	const Element br (Type::unary, "br");
	//! <button>
	const Element button (Type::binary, "button");
	//! <canvas> (HTML 5)
	const Element canvas (Type::binary, "canvas");
	//! <caption>
	const Element caption (Type::binary, "caption");
	//! <cite>
	const Element cite (Type::binary, "cite");
	//! <code>
	const Element code (Type::binary, "code");
	//! <col>
	const Element col (Type::unary, "col");
	//! <colgroup>
	const Element colgroup (Type::binary, "colgroup");
	//! <command> (HTML 5)
	const Element command (Type::binary, "command");
	//! <datalist> (HTML 5)
	const Element datalist (Type::binary, "datalist");
	//! <dd>
	const Element dd (Type::binary, "dd");
	//! <del>
	const Element del (Type::binary, "del");
	//! <details> (HTML 5)
	const Element details (Type::binary, "details");
	//! <dfn>
	const Element dfn (Type::binary, "dfn");
	//! <div>
	const Element div (Type::binary, "div");
	//! <dl>
	const Element dl (Type::binary, "dl");
	//! <dt>
	const Element dt (Type::binary, "dt");
	//! <em>
	const Element em (Type::binary, "em");
	//! <embed> (HTML 5)
	const Element embed (Type::unary, "embed");
	//! <fieldset>
	const Element fieldset (Type::binary, "fieldset");
	//! <figcaption> (HTML 5)
	const Element figcaption (Type::binary, "figcaption");
	//! <figure> (HTML 5)
	const Element figure (Type::binary, "figure");
	//! <footer> (HTML 5)
	const Element footer (Type::binary, "footer");
	//! <form>
	const Element form (Type::binary, "form");
	//! <frame> (Frameset DTDs only)
	const Element frame (Type::unary, "frame");
	//! <frameset> (Frameset DTDs only)
	const Element frameset (Type::binary, "frameset");
	//! <h1>
	const Element h1 (Type::binary, "h1");
	//! <h2>
	const Element h2 (Type::binary, "h2");
	//! <h3>
	const Element h3 (Type::binary, "h3");
	//! <h4>
	const Element h4 (Type::binary, "h4");
	//! <h5>
	const Element h5 (Type::binary, "h5");
	//! <h6>
	const Element h6 (Type::binary, "h6");
	//! <head>
	const Element head (Type::binary, "head");
	//! <header> (HTML 5)
	const Element header (Type::binary, "header");
	//! <hgroup> (HTML 5)
	const Element hgroup (Type::binary, "hgroup");
	//! <hr>
	const Element hr (Type::unary, "hr");
	//! <html>
	const Element html (Type::binary, "html");
	//! <i>
	const Element i (Type::binary, "i");
	//! <iframe> (Frameset DTDs only / HTML 5)
	const Element iframe (Type::binary, "iframe");
	//! <img>
	const Element img (Type::unary, "img");
	//! <input>
	const Element input (Type::unary, "input");
	//! <ins>
	const Element ins (Type::binary, "ins");
	//! <keygen> (HTML 5)
	const Element keygen (Type::unary, "keygen");
	//! <kbd>
	const Element kbd (Type::binary, "kbd");
	//! <label>
	const Element label (Type::binary, "label");
	//! <legend>
	const Element legend (Type::binary, "legend");
	//! <li>
	const Element li (Type::binary, "li");
	//! <link>
	const Element link (Type::unary, "link");
	//! <mark> (HTML 5)
	const Element mark (Type::binary, "mark");
	//! <menu>
	const Element menu (Type::binary, "menu");
	//! <meta>
	const Element meta (Type::unary, "meta");
	//! <meter> (HTML 5)
	const Element meter (Type::binary, "meter");
	//! <nav> (HTML 5)
	const Element nav (Type::binary, "nav");
	//! <noframes> (Frameset DTDs only)
	const Element noframes (Type::binary, "noframes");
	//! <noscript>
	const Element noscript (Type::binary, "noscript");
	//! <object>
	const Element object (Type::binary, "object");
	//! <ol>
	const Element ol (Type::binary, "ol");
	//! <optgroup>
	const Element optgroup (Type::binary, "optgroup");
	//! <option>
	const Element option (Type::binary, "option");
	//! <output> (HTML 5)
	const Element output (Type::binary, "output");
	//! <p>
	const Element p (Type::binary, "p");
	//! <param>
	const Element param (Type::unary, "param");
	//! <pre>
	const Element pre (Type::binary, "pre");
	//! <progress> (HTML 5)
	const Element progress (Type::binary, "progress");
	//! <q>
	const Element q (Type::binary, "q");
	//! <rb> (XHTML 1.1 Ruby)
	const Element rb (Type::binary, "rb");
	//! <rb> (XHTML 1.1 Ruby)
	const Element rbc (Type::binary, "rbc");
	//! <rp> (XHTML 1.1 / HTML 5 Ruby)
	const Element rp (Type::binary, "rp");
	//! <rt> (XHTML 1.1 / HTML 5 Ruby)
	const Element rt (Type::binary, "rt");
	//! <rtc> (XHTML 1.1 Ruby)
	const Element rtc (Type::binary, "rtc");
	//! <ruby> (XHTML 1.1 / HTML 5 Ruby)
	const Element ruby (Type::binary, "ruby");
	//! <s>
	const Element s  (Type::binary, "s");
	//! <samp>
	const Element samp (Type::binary, "samp");
	//! <script>
	const Element script (Type::binary, "script");
	//! <section> (HTML 5)
	const Element section (Type::binary, "section"); 
	//! <select>
	const Element select (Type::binary, "select");
	//! <small>
	const Element small (Type::binary, "small");
	//! <source> (HTML 5)
	const Element source (Type::binary, "source"); 
	//! <span>
	const Element span (Type::binary, "span");
	//! <strong>
	const Element strong (Type::binary, "strong");
	//! <style>
	const Element style (Type::binary, "style");
	//! <sub>
	const Element sub (Type::binary, "sub");
	//! <summary> (HTML 5)
	const Element summary (Type::binary, "summary"); 
	//! <sup>
	const Element sup (Type::binary, "sup");
	//! <table>
	const Element table (Type::binary, "table");
	//! <tbody>
	const Element tbody (Type::binary, "tbody");
	//! <td>
	const Element td (Type::binary, "td");
	//! <textarea>
	const Element textarea (Type::binary, "textarea");
	//! <tfoot>
	const Element tfoot (Type::binary, "tfoot");
	//! <th>
	const Element th (Type::binary, "th");
	//! <thead>
	const Element thead (Type::binary, "thread");
	//! <time> (HTML 5)
	const Element time (Type::binary, "time");
	//! <title>
	const Element title (Type::binary, "title");
	//! <tr>
	const Element tr (Type::binary, "tr");
	//! <track> (HTML 5)
	const Element track (Type::unary, "track");
	//! <tt> (not in HTML 5)
	const Element tt (Type::binary, "tt");
	//! <ul> 
	const Element ul (Type::binary, "ul");
	//! <var>
	const Element var (Type::binary, "var");	
	//! <video> (HTML 5)
	const Element video (Type::binary, "video");
	//!  <wbr> (HTML 5)
	const Element wbr (Type::binary, "wbr"); 
	
	//! <!-- ... -->
	const Element comment (Type::comment, "!--");
	
	//! @c wchar_t specialization of Html_begin<T>
	typedef element::HTML_begin<wchar_t> HTML_begin;
	//! @c wchar_t specialization of Html_end<T>
	typedef element::HTML_end<wchar_t> HTML_end;
	//! @c wchar_t specialization of Body_begin<T>
	typedef element::Body_begin<wchar_t> Body_begin;
	//! @c wchar_t specialization of Body_end<T>
	typedef element::Body_end<wchar_t> Body_end;
	//! <html>
	const HTML_begin html_begin;
	//! </html>
	const HTML_end html_end;
	//! <body>
	const Body_begin body_begin;
	//! </body>
	const Body_end body_end;

//@{
//! @c wchar_t specialization of P
std::pair<std::string, std::wstring>
P(const std::string& s1, const std::wstring& s2) {
	return element::P(s1, s2);
}
//! @c wchar_t specialization of P
std::pair<std::string, std::wstring>
P(std::string&& s1, std::wstring&& s2) {
	return element::P(std::move(s1), std::move(s2));
}
//@}
	
}

}
}

MOSH_FCGI_END

#endif
