/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "rendersettings_p.h"

#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/private/abstractrenderer_p.h>
#include <Qt3DRender/private/qrendersettings_p.h>
#include <Qt3DRender/private/qrendercapabilities_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

RenderSettings::RenderSettings()
    : BackendNode()
    , m_renderPolicy(QRenderSettings::OnDemand)
    , m_pickMethod(QPickingSettings::BoundingVolumePicking)
    , m_pickResultMode(QPickingSettings::NearestPick)
    , m_faceOrientationPickingMode(QPickingSettings::FrontFace)
    , m_pickWorldSpaceTolerance(.1f)
    , m_activeFrameGraph()
{
}

void RenderSettings::syncFromFrontEnd(const Qt3DCore::QNode *frontEnd, bool firstTime)
{
    const QRenderSettings *node = qobject_cast<const QRenderSettings *>(frontEnd);
    if (!node)
        return;

    BackendNode::syncFromFrontEnd(frontEnd, firstTime);

    const Qt3DCore::QNodeId activeFGId = Qt3DCore::qIdForNode(node->activeFrameGraph());
    if (activeFGId != m_activeFrameGraph) {
        m_activeFrameGraph = activeFGId;
    }

    if (node->renderPolicy() != m_renderPolicy) {
        m_renderPolicy = node->renderPolicy();
    }

    auto ncnode = const_cast<QRenderSettings *>(node);
    if (ncnode->pickingSettings()->pickMethod() != m_pickMethod) {
        m_pickMethod = ncnode->pickingSettings()->pickMethod();
    }

    if (ncnode->pickingSettings()->pickResultMode() != m_pickResultMode) {
        m_pickResultMode = ncnode->pickingSettings()->pickResultMode();
    }

    if (!qFuzzyCompare(ncnode->pickingSettings()->worldSpaceTolerance(), m_pickWorldSpaceTolerance)) {
        m_pickWorldSpaceTolerance = ncnode->pickingSettings()->worldSpaceTolerance();
    }

    if (ncnode->pickingSettings()->faceOrientationPickingMode() != m_faceOrientationPickingMode) {
        m_faceOrientationPickingMode = ncnode->pickingSettings()->faceOrientationPickingMode();
    }

    if (firstTime)
        m_capabilities = QRenderCapabilitiesPrivate::get(const_cast<QRenderSettings *>(node)->renderCapabilities())->toString();

    // Either because something above as changed or if QRenderSettingsPrivate::invalidFrame()
    // was called
    markDirty(AbstractRenderer::AllDirty);
}

RenderSettingsFunctor::RenderSettingsFunctor(AbstractRenderer *renderer)
    : m_renderer(renderer)
{
}

Qt3DCore::QBackendNode *RenderSettingsFunctor::create(const Qt3DCore::QNodeCreatedChangeBasePtr &change) const
{
    Q_UNUSED(change)
    if (m_renderer->settings() != nullptr) {
        qWarning() << "Renderer settings already exists";
        return nullptr;
    }

    RenderSettings *settings = new RenderSettings;
    settings->setRenderer(m_renderer);
    m_renderer->setSettings(settings);
    return settings;
}

Qt3DCore::QBackendNode *RenderSettingsFunctor::get(Qt3DCore::QNodeId id) const
{
    Q_UNUSED(id)
    return m_renderer->settings();
}

void RenderSettingsFunctor::destroy(Qt3DCore::QNodeId id) const
{
    Q_UNUSED(id)
    // Deletes the old settings object
    auto settings = m_renderer->settings();
    if (settings && settings->peerId() == id) {
        m_renderer->setSettings(nullptr);
        delete settings;
    }
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
