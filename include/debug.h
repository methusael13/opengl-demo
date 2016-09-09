#ifndef __DEBUG__
#define __DEBUG__

/**
 * @author: Methusael Murmu
 */

#define CHECK(condition, label) \
    if (!(condition)) goto label;

#endif
