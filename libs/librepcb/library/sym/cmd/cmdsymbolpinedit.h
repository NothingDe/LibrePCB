/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
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

#ifndef LIBREPCB_LIBRARY_CMDSYMBOLPINEDIT_H
#define LIBREPCB_LIBRARY_CMDSYMBOLPINEDIT_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcb/common/undocommand.h>
#include <librepcb/common/units/all_length_units.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {
namespace library {

class SymbolPin;

/*****************************************************************************************
 *  Class CmdSymbolPinEdit
 ****************************************************************************************/

/**
 * @brief The CmdSymbolPinEdit class
 */
class CmdSymbolPinEdit final : public UndoCommand
{
    public:

        // Constructors / Destructor
        CmdSymbolPinEdit() = delete;
        CmdSymbolPinEdit(const CmdSymbolPinEdit& other) = delete;
        explicit CmdSymbolPinEdit(SymbolPin& pin) noexcept;
        ~CmdSymbolPinEdit() noexcept;

        // Setters
        void setName(const QString& name, bool immediate) noexcept;
        void setLength(const Length& length, bool immediate) noexcept;
        void setPosition(const Point& pos, bool immediate) noexcept;
        void setDeltaToStartPos(const Point& deltaPos, bool immediate) noexcept;
        void setRotation(const Angle& angle, bool immediate) noexcept;
        void rotate(const Angle& angle, const Point& center, bool immediate) noexcept;

        // Operator Overloadings
        CmdSymbolPinEdit& operator=(const CmdSymbolPinEdit& rhs) = delete;


    private:

        // Private Methods

        /// @copydoc UndoCommand::performExecute()
        bool performExecute() override;

        /// @copydoc UndoCommand::performUndo()
        void performUndo() override;

        /// @copydoc UndoCommand::performRedo()
        void performRedo() override;


        // Private Member Variables

        // Attributes from the constructor
        SymbolPin& mPin;

        // General Attributes
        QString mOldName;
        QString mNewName;
        Length mOldLength;
        Length mNewLength;
        Point mOldPos;
        Point mNewPos;
        Angle mOldRotation;
        Angle mNewRotation;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb

#endif // LIBREPCB_LIBRARY_CMDSYMBOLPINEDIT_H
