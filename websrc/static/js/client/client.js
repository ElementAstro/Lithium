/*
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

// ----------------------------------------------------------------
//
// Provide a basic UI configuration
//
// ----------------------------------------------------------------

/**
 * AdminLTE Demo Menu
 * ------------------
 * You should not use this file in production.
 * This file is for demo purposes only.
 */

/* eslint-disable camelcase */

(function ($) {
    'use strict'

    function capitalizeFirstLetter(string) {
        return string.charAt(0).toUpperCase() + string.slice(1)
    }

    function createSkinBlock(colors, callback, noneSelected) {
        var $block = $('<select />', {
            class: noneSelected ? 'custom-select mb-3 border-0' : 'custom-select mb-3 text-light border-0 ' + colors[0].replace(/accent-|navbar-/, 'bg-')
        })

        if (noneSelected) {
            var $default = $('<option />', {
                text: '默认主题'
            })

            $block.append($default)
        }

        colors.forEach(function (color) {
            var $color = $('<option />', {
                class: (typeof color === 'object' ? color.join(' ') : color).replace('navbar-', 'bg-').replace('accent-', 'bg-'),
                text: capitalizeFirstLetter((typeof color === 'object' ? color.join(' ') : color).replace(/navbar-|accent-|bg-/, '').replace('-', ' '))
            })

            $block.append($color)
        })
        if (callback) {
            $block.on('change', callback)
        }

        return $block
    }

    var $sidebar = $('.control-sidebar')
    var $container = $('<div />', {
        class: 'p-3 control-sidebar-content'
    })

    $sidebar.append($container)

    // Checkboxes

    $container.append(
        '<h5>自定义LightAPT</h5><hr class="mb-2"/>'
    )

    // switch light mode or dark mode
    var $dark_mode_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('body').hasClass('dark-mode'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('body').addClass('dark-mode'),
                $('.navbar').addClass('navbar-dark'),
                $('.nav-link').addClass('bg-dark'),
                $('.main-sidebar').addClass('sidebar-dark-primary'),
                $('.content-wrapper').addClass('bg-dark'),
                $('.dropdown-toggle').removeClass('bg-info').addClass('bg-danger'),
                $('.main-footer').addClass('dark-mode'),
                $('.control-sidebar').addClass('control-sidebar-dark')
        } else {
            $('body').removeClass('dark-mode'),
                $('.navbar').removeClass('navbar-dark'),
                $('.nav-link').removeClass('bg-dark')
            $('.main-sidebar').removeClass('sidebar-dark-primary'),
                $('.content-wrapper').removeClass('bg-dark'),
                $('.dropdown-toggle').removeClass('bg-danger').addClass('bg-info'),
                $('.main-footer').removeClass('dark-mode'),
                $('.control-sidebar').removeClass('control-sidebar-dark'),
                $('.card').removeClass('card-primary')
        }
    })
    var $dark_mode_container = $('<div />', { class: 'mb-4' }).append($dark_mode_checkbox).append('<label>暗色主题</lable>')
    $container.append($dark_mode_container)

    $container.append('<h6>侧边栏选项</h6>')

    var $sidebar_collapsed_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('body').hasClass('sidebar-collapse'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('body').addClass('sidebar-collapse')
            $(window).trigger('resize')
        } else {
            $('body').removeClass('sidebar-collapse')
            $(window).trigger('resize')
        }
    })
    var $sidebar_collapsed_container = $('<div />', { class: 'mb-1' }).append($sidebar_collapsed_checkbox).append('<label>收起</label>')
    $container.append($sidebar_collapsed_container)

    $(document).on('collapsed.lte.pushmenu', '[data-widget="pushmenu"]', function () {
        $sidebar_collapsed_checkbox.prop('checked', true)
    })
    $(document).on('shown.lte.pushmenu', '[data-widget="pushmenu"]', function () {
        $sidebar_collapsed_checkbox.prop('checked', false)
    })

    var $sidebar_mini_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('body').hasClass('sidebar-mini'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('body').addClass('sidebar-mini')
        } else {
            $('body').removeClass('sidebar-mini')
        }
    })
    var $sidebar_mini_container = $('<div />', { class: 'mb-1' }).append($sidebar_mini_checkbox).append('<label>侧边栏最小化</label>')
    $container.append($sidebar_mini_container)

    var $flat_sidebar_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('.nav-sidebar').hasClass('nav-flat'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('.nav-sidebar').addClass('nav-flat')
        } else {
            $('.nav-sidebar').removeClass('nav-flat')
        }
    })
    var $flat_sidebar_container = $('<div />', { class: 'mb-1' }).append($flat_sidebar_checkbox).append('<label>侧边栏扁平化</label>')
    $container.append($flat_sidebar_container)

    var $legacy_sidebar_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('.nav-sidebar').hasClass('nav-legacy'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('.nav-sidebar').addClass('nav-legacy')
        } else {
            $('.nav-sidebar').removeClass('nav-legacy')
        }
    })
    var $legacy_sidebar_container = $('<div />', { class: 'mb-1' }).append($legacy_sidebar_checkbox).append('<label>侧边栏传统风格</label>')
    $container.append($legacy_sidebar_container)

    var $compact_sidebar_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('.nav-sidebar').hasClass('nav-compact'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('.nav-sidebar').addClass('nav-compact')
        } else {
            $('.nav-sidebar').removeClass('nav-compact')
        }
    })
    var $compact_sidebar_container = $('<div />', { class: 'mb-1' }).append($compact_sidebar_checkbox).append('<label>侧边栏紧凑</label>')
    $container.append($compact_sidebar_container)

    // Child elements indents
    var $child_indent_sidebar_checkbox = $('<input />', {
        type: 'checkbox',
        value: 1,
        checked: $('.nav-sidebar').hasClass('nav-child-indent'),
        class: 'mr-1'
    }).on('click', function () {
        if ($(this).is(':checked')) {
            $('.nav-sidebar').addClass('nav-child-indent')
        } else {
            $('.nav-sidebar').removeClass('nav-child-indent')
        }
    })
    var $child_indent_sidebar_container = $('<div />', { class: 'mb-1' }).append($child_indent_sidebar_checkbox).append('<label>子元素缩进</label>')
    $container.append($child_indent_sidebar_container)

    // Color Arrays

    var navbar_dark_skins = [
        'navbar-primary',
        'navbar-secondary',
        'navbar-info',
        'navbar-success',
        'navbar-danger',
        'navbar-indigo',
        'navbar-purple',
        'navbar-pink',
        'navbar-navy',
        'navbar-lightblue',
        'navbar-teal',
        'navbar-cyan',
        'navbar-dark',
        'navbar-gray-dark',
        'navbar-gray'
    ]

    var navbar_light_skins = [
        'navbar-light',
        'navbar-warning',
        'navbar-white',
        'navbar-orange'
    ]

    var sidebar_colors = [
        'bg-primary',
        'bg-warning',
        'bg-info',
        'bg-danger',
        'bg-success',
        'bg-indigo',
        'bg-lightblue',
        'bg-navy',
        'bg-purple',
        'bg-fuchsia',
        'bg-pink',
        'bg-maroon',
        'bg-orange',
        'bg-lime',
        'bg-teal',
        'bg-olive'
    ]

    var accent_colors = [
        'accent-primary',
        'accent-warning',
        'accent-info',
        'accent-danger',
        'accent-success',
        'accent-indigo',
        'accent-lightblue',
        'accent-navy',
        'accent-purple',
        'accent-fuchsia',
        'accent-pink',
        'accent-maroon',
        'accent-orange',
        'accent-lime',
        'accent-teal',
        'accent-olive'
    ]

    var sidebar_skins = [
        'sidebar-dark-primary',
        'sidebar-dark-warning',
        'sidebar-dark-info',
        'sidebar-dark-danger',
        'sidebar-dark-success',
        'sidebar-dark-indigo',
        'sidebar-dark-lightblue',
        'sidebar-dark-navy',
        'sidebar-dark-purple',
        'sidebar-dark-fuchsia',
        'sidebar-dark-pink',
        'sidebar-dark-maroon',
        'sidebar-dark-orange',
        'sidebar-dark-lime',
        'sidebar-dark-teal',
        'sidebar-dark-olive',
        'sidebar-light-primary',
        'sidebar-light-warning',
        'sidebar-light-info',
        'sidebar-light-danger',
        'sidebar-light-success',
        'sidebar-light-indigo',
        'sidebar-light-lightblue',
        'sidebar-light-navy',
        'sidebar-light-purple',
        'sidebar-light-fuchsia',
        'sidebar-light-pink',
        'sidebar-light-maroon',
        'sidebar-light-orange',
        'sidebar-light-lime',
        'sidebar-light-teal',
        'sidebar-light-olive'
    ]

    // Navbar Variants

    $container.append('<h6>导航栏配色</h6>')

    var $navbar_variants = $('<div />', {
        class: 'd-flex'
    })
    var navbar_all_colors = navbar_dark_skins.concat(navbar_light_skins)
    var $navbar_variants_colors = createSkinBlock(navbar_all_colors, function () {
        var color = $(this).find('option:selected').attr('class')
        var $main_header = $('.main-header')
        $main_header.removeClass('navbar-dark').removeClass('navbar-light')
        navbar_all_colors.forEach(function (color) {
            $main_header.removeClass(color)
        })

        $(this).removeClass().addClass('custom-select mb-3 text-light border-0 ')

        if (navbar_dark_skins.indexOf(color) > -1) {
            $main_header.addClass('navbar-dark')
            $(this).addClass(color).addClass('text-light')
        } else {
            $main_header.addClass('navbar-light')
            $(this).addClass(color)
        }

        $main_header.addClass(color)
    })

    var active_navbar_color = null
    $('.main-header')[0].classList.forEach(function (className) {
        if (navbar_all_colors.indexOf(className) > -1 && active_navbar_color === null) {
            active_navbar_color = className.replace('navbar-', 'bg-')
        }
    })

    $navbar_variants_colors.find('option.' + active_navbar_color).prop('selected', true)
    $navbar_variants_colors.removeClass().addClass('custom-select mb-3 text-light border-0 ').addClass(active_navbar_color)

    $navbar_variants.append($navbar_variants_colors)

    $container.append($navbar_variants)

    // Sidebar Colors

    $container.append('<h6>组件配色</h6>')
    var $accent_variants = $('<div />', {
        class: 'd-flex'
    })
    $container.append($accent_variants)
    $container.append(createSkinBlock(accent_colors, function () {
        var color = $(this).find('option:selected').attr('class')
        var $body = $('body')
        accent_colors.forEach(function (skin) {
            $body.removeClass(skin)
        })

        var accent_color_class = color.replace('bg-', 'accent-')

        $body.addClass(accent_color_class)
    }, true))

    var active_accent_color = null
    $('body')[0].classList.forEach(function (className) {
        if (accent_colors.indexOf(className) > -1 && active_accent_color === null) {
            active_accent_color = className.replace('navbar-', 'bg-')
        }
    })

    // $accent_variants.find('option.' + active_accent_color).prop('selected', true)
    // $accent_variants.removeClass().addClass('custom-select mb-3 text-light border-0 ').addClass(active_accent_color)

    $container.append('<h6>侧边栏配色</h6>')
    var $sidebar_variants_dark = $('<div />', {
        class: 'd-flex'
    })
    $container.append($sidebar_variants_dark)
    var $sidebar_dark_variants = createSkinBlock(sidebar_colors, function () {
        var color = $(this).find('option:selected').attr('class')
        if ($('body').hasClass('dark-mode')) {
            var sidebar_class = 'sidebar-dark-' + color.replace('bg-', '')
        }
        else {
            var sidebar_class = 'sidebar-' + color.replace('bg-', '')
        }

        var $sidebar = $('.main-sidebar')
        sidebar_skins.forEach(function (skin) {
            $sidebar.removeClass(skin)
        })

        $(this).removeClass().addClass('custom-select mb-3 text-light border-0').addClass(color)

        $sidebar.addClass(sidebar_class)
        $('.sidebar').removeClass('os-theme-dark').addClass('os-theme-light')
    }, true)
    $container.append($sidebar_dark_variants)

    var active_sidebar_dark_color = null
    $('.main-sidebar')[0].classList.forEach(function (className) {
        var color = className.replace('sidebar-dark-', 'bg-')
        if (sidebar_colors.indexOf(color) > -1 && active_sidebar_dark_color === null) {
            active_sidebar_dark_color = color
        }
    })

    $sidebar_dark_variants.find('option.' + active_sidebar_dark_color).prop('selected', true)
    $sidebar_dark_variants.removeClass().addClass('custom-select mb-3 text-light border-0 ').addClass(active_sidebar_dark_color)

    var logo_skins = navbar_all_colors
    $container.append('<h6>Logo配色</h6>')
    var $logo_variants = $('<div />', {
        class: 'd-flex'
    })
    $container.append($logo_variants)
    var $clear_btn = $('<a />', {
        href: '#'
    }).text('clear').on('click', function (e) {
        e.preventDefault()
        var $logo = $('.brand-link')
        logo_skins.forEach(function (skin) {
            $logo.removeClass(skin)
        })
    })

    var $brand_variants = createSkinBlock(logo_skins, function () {
        var color = $(this).find('option:selected').attr('class')
        var $logo = $('.brand-link')

        if (color === 'navbar-light' || color === 'navbar-white') {
            $logo.addClass('text-black')
        } else {
            $logo.removeClass('text-black')
        }

        logo_skins.forEach(function (skin) {
            $logo.removeClass(skin)
        })

        if (color) {
            $(this).removeClass().addClass('custom-select mb-3 border-0').addClass(color).addClass(color !== 'navbar-light' && color !== 'navbar-white' ? 'text-light' : '')
        } else {
            $(this).removeClass().addClass('custom-select mb-3 border-0')
        }

        $logo.addClass(color)
    }, true).append($clear_btn)
    $container.append($brand_variants)

    var active_brand_color = null
    $('.brand-link')[0].classList.forEach(function (className) {
        if (logo_skins.indexOf(className) > -1 && active_brand_color === null) {
            active_brand_color = className.replace('navbar-', 'bg-')
        }
    })

    if (active_brand_color) {
        $brand_variants.find('option.' + active_brand_color).prop('selected', true)
        $brand_variants.removeClass().addClass('custom-select mb-3 text-light border-0 ').addClass(active_brand_color)
    }
})(jQuery)
