/*
 * Copyright 2004-2006, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
 *
 * This file is part of libtiled-java.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package tiled.io.xml;

import java.lang.String;
import java.io.Writer;
import java.io.IOException;
import java.util.Stack;

/**
 * A simple helper class to write an XML file, based on
 * http://www.xmlsoft.org/html/libxml-xmlwriter.html
 */
public class XMLWriter
{
    private boolean bIndent = true;
    private String indentString = " ";
    private String newLine = "\n";
    private final Writer w;

    private final Stack<String> openElements;
    private boolean bStartTagOpen;
    private boolean bDocumentOpen;


    public XMLWriter(Writer writer) {
        openElements = new Stack<String>();
        w = writer;
    }


    public void setIndent(boolean bIndent) {
        this.bIndent = bIndent;
        newLine = bIndent ? "\n" : "";
    }

    public void setIndentString(String indentString) {
        this.indentString = indentString;
    }


    public void startDocument() throws IOException {
        startDocument("1.0");
    }

    public void startDocument(String version) throws IOException {
        w.write("<?xml version=\"" + version + "\" encoding=\"UTF-8\"?>"
                + newLine);
        bDocumentOpen = true;
    }

    public void writeDocType(String name, String pubId, String sysId)
            throws IOException, XMLWriterException {
        if (!bDocumentOpen) {
            throw new XMLWriterException(
                    "Can't write DocType, no open document.");
        } else if (!openElements.isEmpty()) {
            throw new XMLWriterException(
                    "Can't write DocType, open elements exist.");
        }

        w.write("<!DOCTYPE " + name + " ");

        if (pubId != null) {
            w.write("PUBLIC \"" + pubId + "\"");
            if (sysId != null) {
                w.write(" \"" + sysId + "\"");
            }
        } else if (sysId != null) {
            w.write("SYSTEM \"" + sysId + "\"");
        }

        w.write(">" + newLine);
    }

    public void startElement(String name)
        throws IOException, XMLWriterException {
        if (!bDocumentOpen) {
            throw new XMLWriterException(
                    "Can't start new element, no open document.");
        }

        if (bStartTagOpen) {
            w.write(">" + newLine);
        }

        writeIndent();
        w.write("<" + name);

        openElements.push(name);
        bStartTagOpen = true;
    }


    public void endDocument() throws IOException {
        // End all open elements.
        while (!openElements.isEmpty()) {
            endElement();
        }
        
        w.flush(); //writers do not always flush automatically...
    }

    public void endElement() throws IOException {
        String name = openElements.pop();

        // If start tag still open, end with />, else with </name>.
        if (bStartTagOpen) {
            w.write("/>" + newLine);
            bStartTagOpen = false;
        } else {
            writeIndent();
            w.write("</" + name + ">" + newLine);
        }

        // Set document closed when last element is closed
        if (openElements.isEmpty()) {
            bDocumentOpen = false;
        }
    }


    public void writeAttribute(String name, String content)
        throws IOException, XMLWriterException {
        if (bStartTagOpen) {
            String escapedContent = (content != null) ?
                    content.replaceAll("\"", "&quot;") : "";
            w.write(" " + name + "=\"" + escapedContent + "\"");
        } else {
            throw new XMLWriterException(
                    "Can't write attribute without open start tag.");
        }
    }

    public void writeAttribute(String name, int content)
        throws IOException, XMLWriterException {
        writeAttribute(name, String.valueOf(content));
    }

    public void writeAttribute(String name, float content)
        throws IOException, XMLWriterException {
        writeAttribute(name, String.valueOf(content));
    }

    public void writeCDATA(String content) throws IOException {
        if (bStartTagOpen) {
            w.write(">" + newLine);
            bStartTagOpen = false;
        }

        writeIndent();
        w.write(content + newLine);
    }

    public void writeComment(String content) throws IOException {
        if (bStartTagOpen) {
            w.write(">" + newLine);
            bStartTagOpen = false;
        }

        writeIndent();
        w.write("<!-- " + content + " -->" + newLine);
    }

    public void writeElement(String name, String content)
        throws IOException, XMLWriterException {
        startElement(name);
        writeCDATA(content);
        endElement();
    }


    private void writeIndent() throws IOException {
        if (bIndent) {
            for (int i = 0; i < openElements.size(); i++) {
                w.write(indentString);
            }
        }
    }
}
