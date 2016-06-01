/*
  Copyright 2012, Robert Knight

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
*/

#pragma once

#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QVariant>

#if __cplusplus >= 201103L
#include <functional> /* for std::function */
#endif

namespace Mustache
{

class PartialResolver;
class Renderer;

/** Context is an interface that Mustache::Renderer::render() uses to
  * fetch substitutions for template tags.
  */
class Context
{
public:
	/** Create a context.  @p resolver is used to fetch the expansions for any {{>partial}} tags
	  * which appear in a template.
	  */
	explicit Context(PartialResolver* resolver = 0);
	virtual ~Context() {}

	/** Returns a string representation of the value for @p key in the current context.
	  * This is used to replace a Mustache value tag.
	  */
	virtual QString stringValue(const QString& key) const = 0;

	/** Returns true if the value for @p key is 'false' or an empty list.
	  * 'False' values typically include empty strings, the boolean value false etc.
	  *
	  * When processing a section Mustache tag, the section is not rendered if the key
	  * is false, or for an inverted section tag, the section is only rendered if the key
	  * is false.
	  */
	virtual bool isFalse(const QString& key) const = 0;

	/** Returns the number of items in the list value for @p key or 0 if
	  * the value for @p key is not a list.
	  */
	virtual int listCount(const QString& key) const = 0;

	/** Set the current context to the value for @p key.
	  * If index is >= 0, set the current context to the @p index'th value
	  * in the list value for @p key.
	  */
	virtual void push(const QString& key, int index = -1) = 0;

	/** Exit the current context. */
	virtual void pop() = 0;

	/** Returns the partial template for a given @p key. */
	QString partialValue(const QString& key) const;

	/** Returns the partial resolver passed to the constructor. */
	PartialResolver* partialResolver() const;

	/** Returns true if eval() should be used to render section tags using @p key.
	 * If canEval() returns true for a key, the renderer will pass the literal, unrendered
	 * block of text for the section to eval() and replace the section with the result.
	 *
	 * canEval() and eval() are equivalents for callable objects (eg. lambdas) in other
	 * Mustache implementations.
	 *
	 * The default implementation always returns false.
	 */
	virtual bool canEval(const QString& key) const;

	/** Callback used to render a template section with the given @p key.
	 * @p renderer will substitute the original section tag with the result of eval().
	 *
	 * The default implementation returns an empty string.
	 */
	virtual QString eval(const QString& key, const QString& _template, Renderer* renderer);

private:
	PartialResolver* m_partialResolver;
};

/** A context implementation which wraps a QVariantHash or QVariantMap. */
class QtVariantContext : public Context
{
public:
	/** Construct a QtVariantContext which wraps a dictionary in a QVariantHash
	 * or a QVariantMap.
	 */
#if __cplusplus >= 201103L
	typedef std::function<QString(const QString&, Mustache::Renderer*, Mustache::Context*)> fn_t;
#else
	typedef QString (*fn_t)(const QString&, Mustache::Renderer*, Mustache::Context*);
#endif
	explicit QtVariantContext(const QVariant& root, PartialResolver* resolver = 0);

	virtual QString stringValue(const QString& key) const;
	virtual bool isFalse(const QString& key) const;
	virtual int listCount(const QString& key) const;
	virtual void push(const QString& key, int index = -1);
	virtual void pop();
	virtual bool canEval(const QString& key) const;
	virtual QString eval(const QString& key, const QString& _template, Mustache::Renderer* renderer);

private:
	QVariant value(const QString& key) const;

	QStack<QVariant> m_contextStack;
};

/** Interface for fetching template partials. */
class PartialResolver
{
public:
	virtual ~PartialResolver() {}

	/** Returns the partial template with a given @p name. */
	virtual QString getPartial(const QString& name) = 0;
};

/** A simple partial fetcher which returns templates from a map of (partial name -> template)
  */
class PartialMap : public PartialResolver
{
public:
	explicit PartialMap(const QHash<QString,QString>& partials);

	virtual QString getPartial(const QString& name);

private:
	QHash<QString, QString> m_partials;
};

/** A partial fetcher when loads templates from '<name>.mustache' files
 * in a given directory.
 *
 * Once a partial has been loaded, it is cached for future use.
 */
class PartialFileLoader : public PartialResolver
{
public:
	explicit PartialFileLoader(const QString& basePath);

	virtual QString getPartial(const QString& name);

private:
	QString m_basePath;
	QHash<QString, QString> m_cache;
};

/** Holds properties of a tag in a mustache template. */
struct Tag
{
	enum Type
	{
		Null,
		Value, /// A {{key}} or {{{key}}} tag
		SectionStart, /// A {{#section}} tag
		InvertedSectionStart, /// An {{^inverted-section}} tag
		SectionEnd, /// A {{/section}} tag
		Partial, /// A {{^partial}} tag
		Comment, /// A {{! comment }} tag
		SetDelimiter /// A {{=<% %>=}} tag
	};

	enum EscapeMode
	{
		Escape,
		Unescape,
		Raw
	};

	Tag()
		: type(Null)
		, start(0)
		, end(0)
		, escapeMode(Escape)
	{}

	Type type;
	QString key;
	int start;
	int end;
	EscapeMode escapeMode;
};

/** Renders Mustache templates, replacing mustache tags with
  * values from a provided context.
  */
class Renderer
{
public:
	Renderer();

	/** Render a Mustache template, using @p context to fetch
	  * the values used to replace Mustache tags.
	  */
	QString render(const QString& _template, Context* context);

	/** Returns a message describing the last error encountered by the previous
	  * render() call.
	  */
	QString error() const;

	/** Returns the position in the template where the last error occurred
	  * when rendering the template or -1 if no error occurred.
	  *
	  * If the error occurred in a partial template, the returned position is the offset
	  * in the partial template.
	  */
	int errorPos() const;

	/** Returns the name of the partial where the error occurred, or an empty string
	 * if the error occurred in the main template.
	 */
	QString errorPartial() const;

	/** Sets the default tag start and end markers.
	  * This can be overridden within a template.
	  */
	void setTagMarkers(const QString& startMarker, const QString& endMarker);

private:
	QString render(const QString& _template, int startPos, int endPos, Context* context);

	Tag findTag(const QString& content, int pos, int endPos);
	Tag findEndTag(const QString& content, const Tag& startTag, int endPos);
	void setError(const QString& error, int pos);

	void readSetDelimiter(const QString& content, int pos, int endPos);
	static QString readTagName(const QString& content, int pos, int endPos);

	/** Expands @p tag to fill the line, but only if it is standalone.
	 *
	 * The start position is moved to the beginning of the line. The end position is
	 * moved to one past the end of the line. If @p tag is not standalone, it is
	 * left unmodified.
	 *
	 * A tag is standalone if it is the only non-whitespace token on the the line.
	 */
	static void expandTag(Tag& tag, const QString& content);

	QStack<QString> m_partialStack;
	QString m_error;
	int m_errorPos;
	QString m_errorPartial;

	QString m_tagStartMarker;
	QString m_tagEndMarker;

	QString m_defaultTagStartMarker;
	QString m_defaultTagEndMarker;
};

/** A convenience function which renders a template using the given data. */
QString renderTemplate(const QString& templateString, const QVariantHash& args);

};

Q_DECLARE_METATYPE(Mustache::QtVariantContext::fn_t)
