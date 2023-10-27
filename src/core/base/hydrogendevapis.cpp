
/** \section IUFindS */

/* find a member of an IText vector, else NULL */
std::shared_ptr<IText> IUFindTextS(std::shared_ptr<ITextVectorProperty> tvp, const char *name)
{
    for (int i = 0; i < tvp->ntp; i++)
        if (strcmp(tvp->tp[i].name, name) == 0)
            return (&tvp->tp[i]);
    fprintf(stderr, "No IText '%s' in %s.%s\n", name, tvp->device, tvp->name);
    return nullptr;
}

/* find a member of an INumber vector, else NULL */
std::shared_ptr<INumber> IUFindNumberS(std::shared_ptr<INumberVectorProperty> nvp, const char *name)
{
    for (int i = 0; i < nvp->nnp; i++)
        if (strcmp(nvp->np[i].name, name) == 0)
            return (&nvp->np[i]);
    fprintf(stderr, "No INumber '%s' in %s.%s\n", name, nvp->device, nvp->name);
    return nullptr;
}

/* find a member of an ISwitch vector, else NULL */
std::shared_ptr<ISwitch> IUFindSwitchS(std::shared_ptr<ISwitchVectorProperty> svp, const char *name)
{
    for (int i = 0; i < svp->nsp; i++)
        if (strcmp(svp->sp[i].name, name) == 0)
            return (&svp->sp[i]);
    fprintf(stderr, "No ISwitch '%s' in %s.%s\n", name, svp->device, svp->name);
    return nullptr;
}

/* find a member of an ILight vector, else NULL */
std::shared_ptr<ILight> IUFindLightS(std::shared_ptr<ILightVectorProperty> lvp, const char *name)
{
    for (int i = 0; i < lvp->nlp; i++)
        if (strcmp(lvp->lp[i].name, name) == 0)
            return (&lvp->lp[i]);
    fprintf(stderr, "No ILight '%s' in %s.%s\n", name, lvp->device, lvp->name);
    return nullptr;
}

/* find a member of an IBLOB vector, else NULL */
std::shared_ptr<IBLOB> IUFindBLOBS(std::shared_ptr<IBLOBVectorProperty> bvp, const char *name)
{
    for (int i = 0; i < bvp->nbp; i++)
        if (strcmp(bvp->bp[i].name, name) == 0)
            return (&bvp->bp[i]);
    fprintf(stderr, "No IBLOB '%s' in %s.%s\n", name, bvp->device, bvp->name);
    return nullptr;
}

/* find an ON member of an ISwitch vector, else NULL.
 * N.B. user must make sense of result with ISRule in mind.
 */
std::shared_ptr<ISwitch> IUFindOnSwitchS(std::shared_ptr<ISwitchVectorProperty> svp)
{
    for (int i = 0; i < svp->nsp; i++)
        if (svp->sp[i].s == ISS_ON)
            return (&svp->sp[i]);
    /*fprintf(stderr, "No ISwitch On in %s.%s\n", svp->device, svp->name);*/
    return nullptr;
}

/* Find index of the ON member of an ISwitchVectorProperty */
int IUFindOnSwitchIndexS(std::shared_ptr<ISwitchVectorProperty> svp)
{
    for (int i = 0; i < svp->nsp; i++)
        if (svp->sp[i].s == ISS_ON)
            return i;
    return -1;
}

/* Find name the ON member in the given states and names */
const char *IUFindOnSwitchNameS(std::shared_ptr<ISState> states, char *names[], int n)
{
    for (int i = 0; i < n; i++)
        if (states[i] == ISS_ON)
            return names[i];
    return nullptr;
}
