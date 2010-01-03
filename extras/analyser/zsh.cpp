/*****************************************************************************
 * zsh.cpp: create zsh completion rule for vlc
 *****************************************************************************
 * Copyright © 2005-2008 the VideoLAN team
 * $Id$
 *
 * Authors: Sigmund Augdal Helberg <dnumgis@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <stdio.h>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
typedef std::multimap<std::string, std::string> mumap;
typedef std::multimap<int, std::string> mcmap;

typedef std::pair<std::string, std::string> mpair;
typedef std::pair<int, std::string> mcpair;

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc/vlc.h>

/* evil hack */
#undef __PLUGIN__
#undef __BUILTIN__
#include <../src/modules/modules.h>

void ParseModules( mumap &mods, mcmap &mods2 );
void PrintModuleList( mumap &mods, mcmap &mods2 );
void ParseOption( module_config_t *p_item, mumap &mods, mcmap &mods2 );
void PrintOption( char *psz_option, char i_short, char *psz_exlusive,
                   char *psz_text, char *psz_longtext, char *psz_args );

int main( int i_argc, const char **ppsz_argv )
{
    mumap mods;
    mcmap mods2;
    /* Create a libvlc structure */

    libvlc_exception_t ex;
    libvlc_exception_init(&ex);

    const char *argv[i_argc + 1];
    argv[0] = "vlc";
    for( int i = 0; i < i_argc; i++ )
        argv[i+1] = ppsz_argv[i];
    libvlc_instance_t *p_libvlc_instance = libvlc_new(i_argc+1, argv, &ex);

    if( !p_libvlc_instance || libvlc_exception_raised(&ex) )
    {
        libvlc_exception_clear(&ex);
        return 1;
    }

    printf("#compdef vlc\n\n"

           "#This file is autogenerated by zsh.cpp\n"
           "typeset -A opt_args\n"
           "local context state line ret=1\n"
           "local modules\n\n" );

    PrintModuleList( mods, mods2 );

    printf( "_arguments -S -s \\\n" );
    ParseModules( mods, mods2 );
    printf( "  \"(--module)-p[print help on module]:print help on module:($modules)\"\\\n" );
    printf( "  \"(-p)--module[print help on module]:print help on module:($modules)\"\\\n" );
    printf( "  \"(--help)-h[print help]\"\\\n" );
    printf( "  \"(-h)--help[print help]\"\\\n" );
    printf( "  \"(--longhelp)-H[print detailed help]\"\\\n" );
    printf( "  \"(-H)--longhelp[print detailed help]\"\\\n" );
    printf( "  \"(--list)-l[print a list of available modules]\"\\\n" );
    printf( "  \"(-l)--list[print a list of available modules]\"\\\n" );
    printf( "  \"--save-config[save the current command line options in the config file]\"\\\n" );
    printf( "  \"--reset-config[reset the current config to the default values]\"\\\n" );
    printf( "  \"--config[use alternate config file]\"\\\n" );
    printf( "  \"--reset-plugins-cache[resets the current plugins cache]\"\\\n" );
    printf( "  \"--version[print version information]\"\\\n" );
    printf( "  \"*:Playlist item:->mrl\" && ret=0\n\n" );

    printf( "case $state in\n" );
    printf( "  mrl)\n" );
    printf( "    _alternative 'files:file:_files' 'urls:URL:_urls' && ret=0\n" );
    printf( "  ;;\n" );
    printf( "esac\n\n" );

    printf( "return ret\n" );

    libvlc_release( p_libvlc_instance );

    return 0;
}

void ParseModules( mumap &mods, mcmap &mods2 )
{
    module_t       **p_list;
    module_t        *p_module;
    module_config_t *p_item;
    int              i_index;
    int              i_items;
    size_t           i_modules;

    /* List the plugins */
    p_list = module_list_get(&i_modules);
    if( !p_list ) return;
    for( i_index = 0; i_index < i_modules; i_index++ )
    {
        p_module = p_list[i_index];

        /* Exclude empty plugins (submodules don't have config options, they
         * are stored in the parent module) */
        if( p_module->b_submodule )
              continue;
//            p_item = ((module_t *)p_module->p_parent)->p_config;
        else
            p_item = p_module->p_config;

//        printf( "  #%s\n", p_module->psz_longname );
        if( !p_item ) continue;
        i_items = 0;
        do
        {
            if( p_item->i_type == CONFIG_CATEGORY )
            {
//                printf( "  #Category %d\n", p_item->i_value );
            }
            else if( p_item->i_type == CONFIG_SUBCATEGORY )
            {
//                printf( "  #Subcategory %d\n", p_item->i_value );
            }
            if( p_item->i_type & CONFIG_ITEM )
                ParseOption( p_item, mods, mods2 );
        }
        while( i_items++ < p_module->i_config_items && p_item++ );

    }
    module_list_free( p_list );
}

void PrintModuleList( mumap &mods, mcmap &mods2 )
{
    module_t       **p_list = NULL;
    module_t        *p_module;
    int              i_index;
    int              i_items;
    size_t           i_modules;

    /* List the plugins */
    p_list = module_list_get(&i_modules);
    if( !p_list ) return;

    printf( "modules=\"" );
    for( i_index = 0; i_index < i_modules; i_index++ )
    {
        p_module = p_list[i_index];

        /* Exclude empty plugins (submodules don't have config options, they
         * are stored in the parent module) */

        if( strcmp( p_module->psz_object_name, "main" ) )
        {
            mods.insert( mpair( p_module->psz_capability,
                                p_module->psz_object_name ) );
            module_config_t *p_config = p_module->p_config;
            i_items = 0;
            if( p_config ) do
            {
                /* Hack: required subcategory is stored in i_min */
                if( p_config->i_type == CONFIG_SUBCATEGORY )
                {
                    mods2.insert( mcpair( p_config->value.i,
                                          p_module->psz_object_name ) );
                }
            } while( i_items++ < p_module->i_config_items && p_config++ );
            if( p_module->b_submodule )
                continue;
            printf( "%s ", p_module->psz_object_name );
        }

    }
    printf( "\"\n\n" );
    module_list_free( p_list );
    return;
}

void ParseOption( module_config_t *p_item, mumap &mods, mcmap &mods2 )
{
    char *psz_arguments = NULL;
    char *psz_exclusive;
    char *psz_option;
    char *psz_name;
    char *psz_text;
    char *psz_longtext;

#define DUP( x ) strdup( x ? x : "" )

    //Skip deprecated options
    if( p_item->b_removed )
        return;

    switch( p_item->i_type )
    {
    case CONFIG_ITEM_MODULE:
    {
        std::pair<mumap::iterator, mumap::iterator> range = mods.equal_range( p_item->psz_type );
        std::string list = (*range.first).second;
        ++range.first;
        while( range.first != range.second )
        {
            list = list.append( " " );
            list = list.append( range.first->second );
            ++range.first;
        }
        asprintf( &psz_arguments, "(%s)", list.c_str() );
    }
    break;
    case CONFIG_ITEM_MODULE_CAT:
    {
        std::pair<mcmap::iterator, mcmap::iterator> range =
            mods2.equal_range( p_item->min.i );
        std::string list = (*range.first).second;
        ++range.first;
        while( range.first != range.second )
        {
            list = list.append( " " );
            list = list.append( range.first->second );
            ++range.first;
        }
        asprintf( &psz_arguments, "(%s)", list.c_str() );
    }
    break;
    case CONFIG_ITEM_MODULE_LIST_CAT:
    {
        std::pair<mcmap::iterator, mcmap::iterator> range =
            mods2.equal_range( p_item->min.i );
        std::string list = "_values -s , ";
        list = list.append( p_item->psz_name );
        while( range.first != range.second )
        {
            list = list.append( " '*" );
            list = list.append( range.first->second );
            list = list.append( "'" );
            ++range.first;
        }
        psz_arguments = strdup( list.c_str() );
    }
    break;

    case CONFIG_ITEM_STRING:
        if( p_item->i_list )
        {
            int i = p_item->i_list -1;
            char *psz_list;
            if( p_item->ppsz_list_text )
                asprintf( &psz_list, "%s\\:%s", p_item->ppsz_list[i],
                          p_item->ppsz_list_text[i] );
            else
                psz_list = strdup(p_item->ppsz_list[i]);
            char *psz_list2;
            while( i>1 )
            {
                if( p_item->ppsz_list_text )
                    asprintf( &psz_list2, "%s\\:%s %s", p_item->ppsz_list[i-1],
                              p_item->ppsz_list_text[i-1], psz_list );
                else
                    asprintf( &psz_list2, "%s %s", p_item->ppsz_list[i-1],
                              psz_list );

                free( psz_list );
                psz_list = psz_list2;
                i--;
            }
            if( p_item->ppsz_list_text )
                asprintf( &psz_arguments, "((%s))", psz_list );
            else
                asprintf( &psz_arguments, "(%s)", psz_list );

            free( psz_list );
        }
        break;

    case CONFIG_ITEM_FILE:
        psz_arguments = strdup( "_files" );
        break;
    case CONFIG_ITEM_DIRECTORY:
        psz_arguments = strdup( "_files -/" );
        break;

    case CONFIG_ITEM_INTEGER:
        if( p_item->i_list )
        {
            int i = p_item->i_list -1;
            char *psz_list;
            if( p_item->ppsz_list_text )
                asprintf( &psz_list, "%d\\:%s", p_item->pi_list[i],
                          p_item->ppsz_list_text[i] );
            else
                psz_list = strdup(p_item->ppsz_list[i]);
            char *psz_list2;
            while( i>1 )
            {
                if( p_item->ppsz_list_text )
                    asprintf( &psz_list2, "%d\\:%s %s", p_item->pi_list[i-1],
                              p_item->ppsz_list_text[i-1], psz_list );
                else
                    asprintf( &psz_list2, "%s %s", p_item->ppsz_list[i-1],
                              psz_list );

                free( psz_list );
                psz_list = psz_list2;
                i--;
            }
            if( p_item->ppsz_list_text )
                asprintf( &psz_arguments, "((%s))", psz_list );
            else
                asprintf( &psz_arguments, "(%s)", psz_list );

            free( psz_list );
        }
        else if( p_item->min.i != 0 || p_item->max.i != 0 )
        {
//            p_control = new RangedIntConfigControl( p_this, p_item, parent );
        }
        else
        {
//            p_control = new IntegerConfigControl( p_this, p_item, parent );
        }
        break;

    case CONFIG_ITEM_KEY:
//        p_control = new KeyConfigControl( p_this, p_item, parent );
        break;

    case CONFIG_ITEM_FLOAT:
//        p_control = new FloatConfigControl( p_this, p_item, parent );
        break;

    case CONFIG_ITEM_BOOL:
//        p_control = new BoolConfigControl( p_this, p_item, parent );
        asprintf( &psz_exclusive, "--no%s --no-%s", p_item->psz_name,
                 p_item->psz_name );
        psz_name = DUP( p_item->psz_name );
        psz_text = DUP( p_item->psz_text );
        psz_longtext = DUP( p_item->psz_longtext );
        PrintOption( psz_name, p_item->i_short, psz_exclusive,
                     psz_text, psz_longtext, psz_arguments );
        free( psz_name );
        free( psz_text );
        free( psz_longtext );
        free( psz_exclusive );
        asprintf( &psz_exclusive, "--no%s --%s", p_item->psz_name,
                 p_item->psz_name );
        asprintf( &psz_option, "no-%s", p_item->psz_name );
        psz_text = DUP( p_item->psz_text );
        psz_longtext = DUP( p_item->psz_longtext );
        PrintOption( psz_option, p_item->i_short, psz_exclusive,
                     psz_text, psz_longtext, psz_arguments );
        free( psz_text );
        free( psz_longtext );
        free( psz_exclusive );
        free( psz_option );
        asprintf( &psz_exclusive, "--no-%s --%s", p_item->psz_name,
                 p_item->psz_name );
        asprintf( &psz_option, "no%s", p_item->psz_name );
        psz_text = DUP( p_item->psz_text );
        psz_longtext = DUP( p_item->psz_longtext );
        PrintOption( psz_option, p_item->i_short, psz_exclusive,
                     psz_text, psz_longtext, psz_arguments );
        free( psz_text );
        free( psz_longtext );
        free( psz_exclusive );
        free( psz_option );
        return;

    case CONFIG_SECTION:
//        p_control = new SectionConfigControl( p_this, p_item, parent );
        break;

    default:
        break;
    }
    psz_name = DUP( p_item->psz_name );
    psz_text = DUP( p_item->psz_text );
    psz_longtext = DUP( p_item->psz_longtext );
    PrintOption( psz_name, p_item->i_short, NULL,
                 psz_text, psz_longtext, psz_arguments );
    free( psz_name );
    free( psz_text );
    free( psz_longtext );
    free( psz_arguments );
}

void PrintOption( char *psz_option, char i_short, char *psz_exclusive,
                   char *psz_text, char *psz_longtext, char *psz_args )
{
    char *foo;
    if( psz_text )
    {
        while( (foo = strchr( psz_text, ':' ))) *foo=';';
        while( (foo = strchr( psz_text, '"' ))) *foo='\'';
    }
    if( psz_longtext )
    {
        while( (foo = strchr( psz_longtext, ':' ))) *foo=';';
        while( (foo = strchr( psz_longtext, '"' ))) *foo='\'';
    }
    if( !psz_longtext ||
        strchr( psz_longtext, '\n' ) ||
        strchr( psz_longtext, '(' ) ) psz_longtext = psz_text;
    if( i_short )
    {
        if( !psz_exclusive )
            printf( "  \"(-%c)--%s%s[%s]", i_short,
                    psz_option, psz_args?"=":"", psz_text );
        else
            printf( "  \"(-%c%s)--%s%s[%s]", i_short, psz_exclusive,
                    psz_option, psz_args?"=":"", psz_text );
        if( psz_args )
            printf( ":%s:%s\"\\\n", psz_longtext, psz_args );
        else
            printf( "\"\\\n" );
        printf( "  \"(--%s%s)-%c[%s]", psz_option, psz_exclusive,
                i_short, psz_text );
        if( psz_args )
            printf( ":%s:%s\"\\\n", psz_longtext, psz_args );
        else
            printf( "\"\\\n" );

    }
    else
    {
        if( psz_exclusive )
            printf( "  \"(%s)--%s%s[%s]", psz_exclusive, psz_option,
                    psz_args?"=":"", psz_text );
        else
            printf( "  \"--%s[%s]", psz_option, psz_text );

        if( psz_args )
            printf( ":%s:%s\"\\\n", psz_longtext, psz_args );
        else
            printf( "\"\\\n" );
    }
}

