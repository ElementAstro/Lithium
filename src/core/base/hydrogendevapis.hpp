
    /** @brief Find an IText member in a vector text property.
     *  @param tvp a pointer to a text vector property.
     *  @param name the name of the member to search for.
     *  @return a pointer to an IText member on match, or NULL if nothing is found.
     */
    extern std::shared_ptr<IText> IUFindTextS(std::shared_ptr<ITextVectorProperty> tvp, const char *name);

    /** @brief Find an INumber member in a number text property.
     *  @param nvp a pointer to a number vector property.
     *  @param name the name of the member to search for.
     *  @return a pointer to an INumber member on match, or NULL if nothing is found.
     */
    extern std::shared_ptr<INumber> IUFindNumberS(std::shared_ptr<INumberVectorProperty> nvp, const char *name);

    /** @brief Find an ISwitch member in a vector switch property.
     *  @param svp a pointer to a switch vector property.
     *  @param name the name of the member to search for.
     *  @return a pointer to an ISwitch member on match, or NULL if nothing is found.
     */
    extern std::shared_ptr<ISwitch> IUFindSwitchS(std::shared_ptr<ISwitchVectorProperty> svp, const char *name);

    /** @brief Find an ILight member in a vector Light property.
     *  @param lvp a pointer to a Light vector property.
     *  @param name the name of the member to search for.
     *  @return a pointer to an ILight member on match, or NULL if nothing is found.
     */
    extern std::shared_ptr<ILight> IUFindLightS(std::shared_ptr<ILightVectorProperty> lvp, const char *name);

    /** @brief Find an IBLOB member in a vector BLOB property.
     *  @param bvp a pointer to a BLOB vector property.
     *  @param name the name of the member to search for.
     *  @return a pointer to an IBLOB member on match, or NULL if nothing is found.
     */
    extern std::shared_ptr<IBLOB> IUFindBLOBS(std::shared_ptr<IBLOBVectorProperty> bvp, const char *name);

    /** @brief Returns the first ON switch it finds in the vector switch property.
     *  @note This is only valid for ISR_1OFMANY mode. That is, when only one switch out of many is allowed to be ON. Do not use this function if you can have multiple ON switches in the same vector property.
     *  @param sp a pointer to a switch vector property.
     *  @return a pointer to the \e first ON ISwitch member if found. If all switches are off, NULL is returned.
     */
    extern std::shared_ptr<ISwitch> IUFindOnSwitchS(std::shared_ptr<ISwitchVectorProperty> svp);

    /** @brief Returns the index of first ON switch it finds in the vector switch property.
     *  @note This is only valid for ISR_1OFMANY mode. That is, when only one switch out of many is allowed to be ON. Do not use this function if you can have multiple ON switches in the same vector property.
     *  @param sp a pointer to a switch vector property.
     *  @return index to the \e first ON ISwitch member if found. If all switches are off, -1 is returned.
     */
    extern int IUFindOnSwitchIndexS(std::shared_ptr<ISwitchVectorProperty> svp);

    /** @brief Returns the name of the first ON switch it finds in the supplied arguments.
     *  @note This is only valid for ISR_1OFMANY mode. That is, when only one switch out of many is allowed to be ON. Do not use this function if you can have multiple ON switches in the same vector property.
     *  @note This is a convience function intended to be used in ISNewSwitch(...) function to find out ON switch name without having to change actual switch state via IUUpdateSwitch(..)
     *  @param states list of switch states passed by ISNewSwitch()
     *  @param names list of switch names passed by ISNewSwitch()
     *  @param n number of switches passed by ISNewSwitch()
     *  @return name of the \e first ON ISwitch member if found. If all switches are off, NULL is returned.
     */
    extern const char *IUFindOnSwitchNameS(std::shared_ptr<ISState> states, char *names[], int n);
