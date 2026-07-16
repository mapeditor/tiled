/*
 * objectgroupmaterial.cpp
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "objectgroupmaterial.h"

#include <QSGTexture>

using namespace TiledQuick;

class ObjectGroupShader : public QSGMaterialShader
{
public:
    ObjectGroupShader()
    {
        setShaderFileName(VertexStage, QStringLiteral(":/objectgroup.vert.qsb"));
        setShaderFileName(FragmentStage, QStringLiteral(":/objectgroup.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *, QSGMaterial *) override
    {
        if (!state.isMatrixDirty())
            return false;

        auto *buffer = state.uniformData()->data();
        auto *ubuf = reinterpret_cast<ObjectGroupUniformBuffer*>(buffer);

        memcpy(buffer + offsetof(ObjectGroupUniformBuffer, matrix),
               state.combinedMatrix().constData(),
               64);

        ubuf->opacity = state.opacity();

        return true;
    }

    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *) override
    {
        if (binding != 1)
            return;

        QSGTexture *tex = static_cast<ObjectGroupMaterial *>(newMaterial)->texture();

        if (tex)
            tex->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());

        *texture = tex;
    }
};

ObjectGroupMaterial::ObjectGroupMaterial()
{
    setFlag(Blending, true);
    setFlag(NoBatching, true);
}

ObjectGroupMaterial::~ObjectGroupMaterial()
{
}

QSGMaterialType *ObjectGroupMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *ObjectGroupMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ObjectGroupShader();
}

int ObjectGroupMaterial::compare(const QSGMaterial *other) const
{
    auto *otherMaterial = static_cast<const ObjectGroupMaterial *>(other);
    if (mTexture == otherMaterial->mTexture)
        return 0;

    return (mTexture < otherMaterial->mTexture) ? -1 : 1;
}

void ObjectGroupMaterial::setTexture(QSGTexture *texture)
{
    mTexture = texture;
}

