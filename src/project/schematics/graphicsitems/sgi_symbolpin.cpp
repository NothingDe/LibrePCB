/*
 * EDA4U - Professional EDA for everyone!
 * Copyright (C) 2013 Urban Bruhin
 * http://eda4u.ubruhin.ch/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/

#include <QtCore>
#include <QtWidgets>
#include <QPrinter>
#include "sgi_symbolpin.h"
#include "../items/si_symbolpin.h"
#include "../items/si_symbol.h"
#include "../schematic.h"
#include "../../project.h"
#include "../../circuit/netsignal.h"
#include "../../circuit/gencompinstance.h"
#include "../../circuit/gencompsignalinstance.h"
#include "../../../common/schematiclayer.h"
#include "../../../workspace/workspace.h"
#include "../../../workspace/settings/workspacesettings.h"
#include "../../../library/symbolpin.h"
#include "../../../library/genericcomponent.h"

namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

SGI_SymbolPin::SGI_SymbolPin(SI_SymbolPin& pin) noexcept :
    SGI_Base(), mPin(pin), mLibPin(pin.getLibPin())
{
    setZValue(Schematic::ZValue_Symbols);
    setFlags(QGraphicsItem::ItemIsSelectable);
    setPos(mLibPin.getPosition().toPxQPointF());
    setRotation(mLibPin.getAngle().toDeg());
    setToolTip(mLibPin.getName() % ": " % mLibPin.getDescription());

    mCircleLayer = getSchematicLayer(SchematicLayer::SymbolPinCircles);
    Q_ASSERT(mCircleLayer);
    mLineLayer = getSchematicLayer(SchematicLayer::SymbolOutlines);
    Q_ASSERT(mLineLayer);
    mTextLayer = getSchematicLayer(SchematicLayer::SymbolPinNames);
    Q_ASSERT(mTextLayer);

    mStaticText.setTextFormat(Qt::PlainText);
    mStaticText.setPerformanceHint(QStaticText::AggressiveCaching);

    mFont.setStyleStrategy(QFont::StyleStrategy(QFont::OpenGLCompatible | QFont::PreferQuality));
    mFont.setStyleHint(QFont::SansSerif);
    mFont.setFamily("Nimbus Sans L");
    mFont.setPixelSize(5);

    mRadiusPx = Length(600000).toPx();

    updateCacheAndRepaint();
}

SGI_SymbolPin::~SGI_SymbolPin() noexcept
{
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void SGI_SymbolPin::updateCacheAndRepaint() noexcept
{
    mShape = QPainterPath();
    mShape.setFillRule(Qt::WindingFill);
    mBoundingRect = QRectF();

    // rotation
    Angle absAngle = mLibPin.getAngle() + mPin.getSymbol().getAngle();
    mRotate180 = (absAngle <= -Angle::deg90() || absAngle > Angle::deg90());

    // circle
    mShape.addEllipse(-mRadiusPx, -mRadiusPx, 2*mRadiusPx, 2*mRadiusPx);
    mBoundingRect = mBoundingRect.united(mShape.boundingRect());

    // line
    QRectF lineRect = QRectF(QPointF(0, 0), Point(0, mLibPin.getLength()).toPxQPointF()).normalized();
    lineRect.adjust(-Length(79375).toPx(), -Length(79375).toPx(), Length(79375).toPx(), Length(79375).toPx());
    mBoundingRect = mBoundingRect.united(lineRect).normalized();

    // text
    qreal x = mLibPin.getLength().toPx() + 4;
    mStaticText.setText(mPin.getDisplayText());
    mStaticText.prepare(QTransform(), mFont);
    mTextOrigin.setX(mRotate180 ? -x-mStaticText.size().width() : x);
    mTextOrigin.setY(-mStaticText.size().height()/2);
    mStaticText.prepare(QTransform().rotate(mRotate180 ? 90 : -90)
                                    .translate(mTextOrigin.x(), mTextOrigin.y()), mFont);
    if (mRotate180)
        mTextBoundingRect = QRectF(mTextOrigin.y(), mTextOrigin.x(), mStaticText.size().height(), mStaticText.size().width()).normalized();
    else
        mTextBoundingRect = QRectF(mTextOrigin.y(), -mTextOrigin.x()-mStaticText.size().width(), mStaticText.size().height(), mStaticText.size().width()).normalized();
    mBoundingRect = mBoundingRect.united(mTextBoundingRect).normalized();

    update();
}

/*****************************************************************************************
 *  Inherited from QGraphicsItem
 ****************************************************************************************/

void SGI_SymbolPin::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    const bool selected = (option->state & QStyle::State_Selected);// || mSymbolGraphicsItem.isSelected();
    const bool deviceIsPrinter = (dynamic_cast<QPrinter*>(painter->device()) != 0);
    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());

    const library::GenCompSignal* genCompSignal = mPin.getGenCompSignal();
    const NetSignal* netsignal = (genCompSignal ? mPin.getGenCompSignalInstance()->getNetSignal() : nullptr);
    bool requiredPin = mPin.getGenCompSignal()->isRequired();

    // draw line
    QPen pen(mLineLayer->getColor(selected), Length(158750).toPx()*lod, Qt::SolidLine, Qt::RoundCap);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawLine(QPointF(0, 0), Point(0, mLibPin.getLength()).toPxQPointF());

    // draw circle
    if ((!deviceIsPrinter) && (!netsignal))
    {
        painter->setPen(QPen(mCircleLayer->getColor(requiredPin), 0));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), mRadiusPx, mRadiusPx);
    }

    // draw text or filled rect
    if (!mStaticText.text().isEmpty())
    {
        if ((deviceIsPrinter) || (lod > 1))
        {
            // draw text
            painter->save();
            painter->rotate(mRotate180 ? 90 : -90);
            painter->setPen(QPen(mTextLayer->getColor(selected), 0));
            painter->setFont(mFont);
            painter->drawStaticText(mTextOrigin, mStaticText);
            painter->restore();
        }
        else
        {
            // draw filled rect
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(mTextLayer->getColor(selected), Qt::Dense5Pattern));
            painter->drawRect(mTextBoundingRect);
        }
    }

#ifdef QT_DEBUG
    if ((!deviceIsPrinter) && (netsignal) && (Workspace::instance().getSettings().getDebugTools()->getShowSymbolPinNetsignals()))
    {
        // draw net signal name
        QFont font;
        font.setStyleStrategy(QFont::StyleStrategy(QFont::OpenGLCompatible | QFont::PreferQuality));
        font.setStyleHint(QFont::TypeWriter);
        font.setFamily("Monospace");
        font.setPixelSize(3);
        painter->setFont(font);
        painter->setPen(QPen(mCircleLayer->getColor(requiredPin), 0));
        painter->save();
        painter->rotate(mRotate180 ? 90 : -90);
        painter->drawText(QRectF(), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextSingleLine | Qt::TextDontClip, netsignal->getName());
        painter->restore();
    }
    if ((!deviceIsPrinter) && (Workspace::instance().getSettings().getDebugTools()->getShowGraphicsItemsBoundingRect()))
    {
        // draw bounding rect
        painter->setPen(QPen(Qt::red, 0));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(mBoundingRect);
    }
#endif
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

SchematicLayer* SGI_SymbolPin::getSchematicLayer(uint id) const noexcept
{
    return mPin.getSymbol().getSchematic().getProject().getSchematicLayer(id);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project