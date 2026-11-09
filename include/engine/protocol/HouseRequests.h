#ifndef NORTH_ISLAND_PROTOCOL_HOUSE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_HOUSE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace protocol {

    /// @brief Builds 62/1: put the furniture of a bag place down at a cell.
    /// @param bagSlot Bag place of the furniture (u8).
    /// @param cellX Cell x (i32).
    /// @param cellY Cell y (i32).
    /// @param cellZ Cell z (i32).
    /// @param facing Facing (u8).
    /// @return The encoded payload.
    core::Bytes placeFurnitureRequest(
        int bagSlot,
        int cellX,
        int cellY,
        int cellZ,
        int facing
    );

    /// @brief Builds 62/2: take a placed piece back into the bag.
    /// @param piece Piece number (u16).
    /// @return The encoded payload.
    core::Bytes takeBackFurnitureRequest(int piece);

    /// @brief Builds 62/3: move or turn a placed piece.
    /// @param piece Piece number (u16).
    /// @param cellX Cell x (i32).
    /// @param cellY Cell y (i32).
    /// @param cellZ Cell z (i32).
    /// @param facing Facing (u8).
    /// @return The encoded payload.
    core::Bytes moveFurnitureRequest(
        int piece,
        int cellX,
        int cellY,
        int cellZ,
        int facing
    );

    /// @brief Builds 62/4: put a tool onto the Work Platform.
    /// @param bagSlot Bag place of the tool (u8).
    /// @param platform Platform piece (u16).
    /// @return The encoded payload.
    core::Bytes toolOnPlatformRequest(int bagSlot, int platform);

    /// @brief Builds 62/5: put the items of a bag place into a recycling bin.
    /// @param bagSlot Bag place (u8).
    /// @param count Item count (u8).
    /// @param bin Bin piece (u16).
    /// @return The encoded payload.
    core::Bytes recycleItemRequest(int bagSlot, int count, int bin);

    /// @brief Builds 62/6: put a placed piece into a recycling bin.
    /// @param piece Piece number (u16).
    /// @param bin Bin piece (u16).
    /// @return The encoded payload.
    core::Bytes recycleFurnitureRequest(int piece, int bin);

    /// @brief Builds 62/10: use the floor item of a bag place for the room.
    /// @param bagSlot Bag place (u8).
    /// @return The encoded payload.
    core::Bytes useFloorItemRequest(int bagSlot);

    /// @brief Builds 62/11: use the wall item of a bag place for the room.
    /// @param bagSlot Bag place (u8).
    /// @return The encoded payload.
    core::Bytes useWallItemRequest(int bagSlot);

    /// @brief Builds 62/57: a Piano key was struck.
    /// @param note Note number (u8, 1..7).
    /// @return The encoded payload.
    core::Bytes pianoKeyRequest(int note);

    /// @brief Builds 23/99: the player sat down at the Piano.
    /// @param piece Piece number (u16).
    /// @return The encoded payload.
    core::Bytes pianoSitRequest(int piece);

    /// @brief Builds 23/72: the player stopped using the piece he sat at (StopUsingUtensil).
    /// @return The encoded payload.
    core::Bytes stopUsingUtensilRequest();

    /// @brief Builds 23/71: the player sat down at a chair, sofa, cushion, carpet, toilet, treadmill or the Lovely Octopus.
    /// @details Original: the arm of SendServerMsg that sends FUsedUtensil and FUsedUtensilSeat after UseUtensil put him there.
    /// @param piece Piece number (u16).
    /// @param seat Seat (u8).
    /// @return The encoded payload.
    core::Bytes sitUtensilRequest(int piece, int seat);

    /// @brief Builds 62/64: the owner lit the New Year Cracker.
    /// @details Original: TGdUtensil.HandleClick.
    /// @param piece Piece number (u16).
    /// @return The encoded payload.
    core::Bytes newYearCrackerRequest(int piece);

    /// @brief Builds 62/65: the owner set the Gugu Clock going.
    /// @details Original: TGdUtensil.HandleClick.
    /// @param piece Piece number (u16).
    /// @return The encoded payload.
    core::Bytes guguClockRequest(int piece);

    /// @brief Builds 62/14: the owner clicked an Elevator or Stairs on the ground floor.
    /// @details Original: TGdUtensil.HandleClick; FMoveFurnIndex is the piece.
    /// @param piece Piece number (u16).
    /// @return The encoded payload.
    core::Bytes stairsRequest(int piece);

    /// @brief Builds 23/55: the owner steps into a tub.
    /// @param ownerId House owner id (i32).
    /// @param piece Tub piece number (u16).
    /// @return The encoded payload.
    core::Bytes batheRequest(std::int32_t ownerId, int piece);

    /// @brief Builds 23/56: the owner gets out of a tub.
    /// @details Original: LeaveUtensilSeat.
    /// @param ownerId House owner id (i32).
    /// @param piece Tub piece number (u16).
    /// @return The encoded payload.
    core::Bytes stopBatheRequest(std::int32_t ownerId, int piece);

    /// @brief Builds 62/56: the owner opens the fish tank (the server answers 23/160 11).
    /// @return The encoded payload.
    core::Bytes openFishTankRequest();

    /// @brief Builds 23/97: a fish tank request (2: a grain dropped at the click).
    /// @param request Request code (u8).
    /// @return The encoded payload.
    core::Bytes fishTankRequest(int request);

    /// @brief Builds 23/98: a fish tank request with a fish kind.
    /// @details Requests: 1 the fish ate a grain, 3 buy, 4 discard, 5 a kind of fish discarded, 6 a grain reached the bottom, 2 a kind bought.
    /// @param request Request code (u8).
    /// @param kind Fish kind (u8).
    /// @return The encoded payload.
    core::Bytes fishTankKindRequest(int request, int kind);

    /// @brief Builds 62/52: put an item into or take one out of a storage piece.
    /// @details Put options are 1, 3, 5, 7, 9 (with the bag place); take options are 2, 4, 6, 8, 10 (the Fridge 1, the Material Cabinet 3,
    /// the Potion Cabinet 5, the Locker 7, the Medicine Case 9; the take is the next option).
    /// @param option Option code (u8).
    /// @param piece Storage piece (u8).
    /// @param place Place in the piece (u8).
    /// @param bagSlot Bag place, only for put options.
    /// @return The encoded payload.
    core::Bytes houseBoxRequest(int option, int piece, int place, std::optional<int> bagSlot = std::nullopt);

    /// @brief Builds 62/61: the Music Center chose the house's song (0: stopped, no name).
    /// @param song Song number (i32).
    /// @param name The song's file name (no length, no end).
    /// @return The encoded payload.
    core::Bytes houseMusicRequest(int song, const std::string& name);

    /// @brief Builds 62/62 (Big Gas Storage 39122) or 62/63 (Gas Storage 39123): bag places of Gas items to put in.
    /// @param small True for the Gas Storage (62/63), false for the Big one (62/62).
    /// @param bagSlots Bag places, one byte each.
    /// @return The encoded payload.
    core::Bytes gasDepositRequest(bool small, const std::vector<int>& bagSlots);

    /// @brief Builds 62/66: a Picture Frame 39153 or 39154 was clicked (the server answers 62/62).
    /// @param piece Piece number (u8).
    /// @return The encoded payload.
    core::Bytes openPortraitRequest(int piece);

}

#endif
