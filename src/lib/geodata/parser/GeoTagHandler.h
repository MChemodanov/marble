/*
    Copyright (C) 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef GeoTagHandler_h
#define GeoTagHandler_h

#include <QtCore/QHash>

namespace Marble
{

class GeoNode;
class GeoParser;

class GeoTagHandler {
public:
    // API to be implemented by child handlers.
    virtual GeoNode* parse(GeoParser&) const = 0;

    typedef QPair<QString, QString> QualifiedName; // Tag Name & Namespace pair

protected: // This base class is not directly constructable nor is it copyable.
    GeoTagHandler();
    virtual ~GeoTagHandler();

private:
    GeoTagHandler(const GeoTagHandler&);
    GeoTagHandler& operator=(const GeoTagHandler&);

private: // Only our registrar is allowed to register tag handlers.
    friend struct GeoTagHandlerRegistrar;
    static void registerHandler(const QualifiedName&, const GeoTagHandler*);

private: // Only our parser is allowed to access tag handlers.
    friend class GeoParser;
    static const GeoTagHandler* recognizes(const QualifiedName&);

private:
    typedef QHash<QualifiedName, const GeoTagHandler*> TagHash;

    static TagHash* tagHandlerHash();
    static TagHash* s_tagHandlerHash;
};

// Helper structure
struct GeoTagHandlerRegistrar {
public:
    GeoTagHandlerRegistrar(const GeoTagHandler::QualifiedName& name, const GeoTagHandler* handler)
    {
        GeoTagHandler::registerHandler(name, handler);
    }
};

// Macros to ease registering new handlers
#define GEODATA_DEFINE_TAG_HANDLER(Module, UpperCaseModule, Name, NameSpace) \
    static GeoTagHandlerRegistrar s_handler##Name##NameSpace(GeoTagHandler::QualifiedName(Module##Tag_##Name, NameSpace), \
                                                             new UpperCaseModule##Name##TagHandler());

}

#endif // GeoTagHandler_h
