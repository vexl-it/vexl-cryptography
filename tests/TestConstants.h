//
// Created by Adam Salih on 05.04.2022.
//

#ifndef TEST_CONSTANTS_H
#define TEST_CONSTANTS_H

static const char password[] = "Password123";

static const char test_message[] = "{\"widget\": {"\
        "    \"debug\": \"on\","\
        "    \"window\": {"\
        "        \"title\": \"sourceMessage\","\
        "        \"name\": \"main_window\","\
        "        \"width\": 500,"\
        "        \"height\": 500"\
        "    },"\
        "    \"image\": { "\
        "        \"src\": \"Images/Sun.png\","\
        "        \"name\": \"sun1\","\
        "        \"hOffset\": 250,"\
        "        \"vOffset\": 250,"\
        "        \"alignment\": \"center\""\
        "    },"\
        "    \"text\": {"\
        "        \"data\": \"Click Here\","\
        "        \"size\": 36,"\
        "        \"style\": \"bold\","\
        "        \"name\": \"text1\","\
        "        \"hOffset\": 250,"\
        "        \"vOffset\": 100,"\
        "        \"alignment\": \"center\","\
        "        \"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\""\
        "    }"\
        "}}";

#endif
