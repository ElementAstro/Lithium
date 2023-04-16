#pragma once

#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Data
{
    int Id;
    std::string Name;  // The name of the object.
    std::string Type;  // The type of the object.
    std::string RA;    // The right ascension coordinate of the object.
    std::string Dec;   // The declination coordinate of the object.
    std::string Const; // The constellation that the object belongs to.
};

namespace OpenAPT::JASX
{
    /**
     * Reads data from the given JSON file and returns it as a vector of Data objects.
     *
     * @param filename The name of the JSON file to read from.
     * @throw std::runtime_error if there is an error reading from the file.
     */
    std::vector<Data> ReadFromJson(const std::string &filename);

    /**
     * Writes the given vector of Data objects to a JSON file.
     *
     * @param data The vector of Data objects to write to the file.
     * @param filename The name of the file to write to.
     * @throw std::runtime_error if there is an error writing to the file.
     */
    void WriteToJson(const std::vector<Data> &data, const std::string &filename);

    /**
     * Inserts a new data entry into the vector of Data objects.
     *
     * @param d The new data entry to insert.
     */
    void InsertData(std::vector<Data> &data, const Data &d);

    /**
     * Deletes a data entry from the vector of Data objects.
     *
     * @param name The name of the data entry to delete.
     */
    void DeleteData(std::vector<Data> &data, const std::string &name);

    /**
     * Sorts the vector of Data objects by name.
     */
    void SortByName(std::vector<Data> &data);

    /**
     * Filters the vector of Data objects by a user-provided predicate function.
     *
     * @param filter A lambda or function pointer that takes a Data object as its parameter and returns a bool indicating whether the object should be kept in the filtered result.
     * @return A vector containing only the data entries that satisfy the provided filter.
     */
    std::vector<Data> FilterBy(const std::vector<Data> &data, std::function<bool(const Data &)> filter);

    /**
     * Searches for data entries in the vector of Data objects by name.
     *
     * @param name The name to search for.
     * @return A vector containing all data entries whose names contain the search query.
     */
    std::vector<Data> SearchByName(const std::vector<Data> &data, const std::string &name);

    /**
     * Searches for data entries in the vector of Data objects by right ascension and declination.
     *
     * @param ra The right ascension of the target object.
     * @param dec The declination of the target object.
     * @param ra_range The maximum allowed deviation in right ascension.
     * @param dec_range The maximum allowed deviation in declination.
     * @return A vector containing all data entries whose coordinates fall within the specified ranges.
     */
    std::vector<Data> SearchByRaDec(const std::vector<Data> &data, std::string ra, std::string dec, double ra_range, double dec_range);

    /**
     * Converts a string representation of a right ascension or declination coordinate to decimal degrees.
     *
     * @param str The string representation of the coordinate to convert.
     * @return The decimal degree equivalent of the input coordinate.
     */
    double ToDecimal(const std::string &str);

} // namespace OpenAPT::ASX

namespace OpenAPT::CASX
{
    /**
     * Reads data from a csv file and stores it in a vector.
     *
     * @param filename The name of the csv file.
     * @return A vector containing all data entries read from the csv file.
     */
    std::vector<Data> ReadFromCsv(const std::string &filename);

    /**
     * Saves the current list of data entries to a csv file.
     *
     * @param data The vector containing the data entries to save.
     * @param filename The name of the csv file to save the data to.
     */
    void SaveToCsv(const std::vector<Data> &data, const std::string &filename);

    /**
     * Sorts the internal vector of data entries by name.
     */
    void SortByName(std::vector<Data> &data);

    /**
     * Filters the internal vector of data entries by a user-provided predicate function.
     *
     * @param filter A lambda or function pointer that takes a Data object as its parameter and returns a bool indicating whether the object should be kept in the filtered result.
     * @return A vector containing only the data entries that satisfy the provided filter.
     */
    std::vector<Data> FilterBy(const std::vector<Data> &data, std::function<bool(const Data &)> filter);

    /**
     * Searches for data entries in the internal vector by name.
     *
     * @param name The name to search for.
     * @return A vector containing all data entries whose names contain the search query.
     */
    std::vector<Data> SearchByName(const std::vector<Data> &data, const std::string &name);

    /**
     * Searches for data entries in the internal vector by right ascension and declination.
     *
     * @param ra The right ascension of the target object.
     * @param dec The declination of the target object.
     * @param ra_range The maximum allowed deviation in right ascension.
     * @param dec_range The maximum allowed deviation in declination.
     * @return A vector containing all data entries whose coordinates fall within the specified ranges.
     */
    std::vector<Data> SearchByRaDec(const std::vector<Data> &data, std::string ra, std::string dec, double ra_range, double dec_range);

    /**
     * Converts a string representation of a right ascension or declination coordinate to decimal degrees.
     *
     * @param str The string representation of the coordinate to convert.
     * @return The decimal degree equivalent of the input coordinate.
     */
    double ToDecimal(const std::string &str);
}