#pragma once
#ifndef LIBXLSXWRITER_H
#define LIBXLSXWRITER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct lxw_workbook lxw_workbook;
typedef struct lxw_worksheet lxw_worksheet;
typedef struct lxw_format lxw_format;

#define LXW_NO_ERROR 0
#define LXW_ERROR_CREATINGZipFile 1
#define LXW_ERROR_CREATING_PACKAGE 2
#define LXW_ERROR_CREATING_XL_WORKSHEET 3
#define LXW_ERROR_NULL_POINTER 4
#define LXW_ERROR_MEMORY_ERROR 5
#define LXW_ERROR_CREATING_FILE 6
#define LXW_ERROR_COULD_NOT_OPEN_ZIP 7
#define LXW_ERROR_COULD_NOT_CLOSE_ZIP 8
#define LXW_ERROR_INVALID_PARAMETER 9
#define LXW_ERROR_SHEETNAME_ALREADY_EXISTS 10
#define LXW_ERROR_SHEETNAME_IS_TOO_LONG 11
#define LXW_ERROR_INVALID_SHEETNAME_CHARACTER 12
#define LXW_ERROR_SHEETNAME_START_OR_END 13
#define LXW_ERROR_WORKBOOK_INIT 14
#define LXW_ERROR_WORKSHEET_INIT 15
#define LXW_ERROR_LARGE_STRING_LEN_THRESHOLD 16
#define LXW_ERROR_LARGE_STRING_LENGTH 17
#define LXW_ERROR_UNSUPPORTED_IMAGE_TYPE 18
#define LXW_ERROR_DUPLICATE_IMAGE_DPI 19
#define LXW_ERROR_IMAGE_DIMENSIONS 20
#define LXW_MAX_ERROR_CODE 20

typedef enum lxw_error {
    LXW_NO_ERROR = 0,
    LXW_ERROR_CREATING_ZIP_FILE = 1,
    LXW_ERROR_CREATING_PACKAGE = 2,
    LXW_ERROR_CREATING_XL_WORKSHEET = 3,
    LXW_ERROR_NULL_POINTER = 4,
    LXW_ERROR_MEMORY_ERROR = 5,
    LXW_ERROR_CREATING_FILE = 6,
    LXW_ERROR_COULD_NOT_OPEN_ZIP = 7,
    LXW_ERROR_COULD_NOT_CLOSE_ZIP = 8,
    LXW_ERROR_INVALID_PARAMETER = 9,
    LXW_ERROR_SHEETNAME_ALREADY_EXISTS = 10,
    LXW_ERROR_SHEETNAME_IS_TOO_LONG = 11,
    LXW_ERROR_INVALID_SHEETNAME_CHARACTER = 12,
    LXW_ERROR_SHEETNAME_START_OR_END = 13,
    LXW_ERROR_WORKBOOK_INIT = 14,
    LXW_ERROR_WORKSHEET_INIT = 15,
    LXW_ERROR_LARGE_STRING_LEN_THRESHOLD = 16,
    LXW_ERROR_LARGE_STRING_LENGTH = 17,
    LXW_ERROR_UNSUPPORTED_IMAGE_TYPE = 18,
    LXW_ERROR_DUPLICATE_IMAGE_DPI = 19,
    LXW_ERROR_IMAGE_DIMENSIONS = 20
} lxw_error;

typedef enum lxw_border_styles {
    LXW_BORDER_NONE,
    LXW_BORDER_THIN,
    LXW_BORDER_MEDIUM,
    LXW_BORDER_THICK,
    LXW_BORDER_DASHED,
    LXW_BORDER_HAIR,
    LXW_BORDER_DOTTED,
    LXW_BORDER_DOUBLE,
    LXW_BORDER_SLANT_DASH_DOT,
    LXW_BORDER_MEDIUM_DASHED,
    LXW_BORDER_MEDIUM_DASH_DOT,
    LXW_BORDER_MEDIUM_DASH_DOT_DOT,
    LXW_BORDER_SLANTED_DASH_DOT
} lxw_border_styles;

typedef enum lxw_border_diagonal_types {
    LXW_BORDER_DIAGONAL_NONE,
    LXW_BORDER_DIAGONAL_UP,
    LXW_BORDER_DIAGONAL_DOWN,
    LXW_BORDER_DIAGONAL_BOTH
} lxw_border_diagonal_types;

typedef enum lxw_vertical_aligns {
    LXW_ALIGN_VERTICAL_TOP,
    LXW_ALIGN_VERTICAL_BOTTOM,
    LXW_ALIGN_VERTICAL_CENTER,
    LXW_ALIGN_VERTICAL_JUSTIFY,
    LXW_ALIGN_VERTICAL_DISTRIBUTED
} lxw_vertical_aligns;

typedef enum lxw_horizontal_aligns {
    LXW_ALIGN_HORIZONTAL_NONE,
    LXW_ALIGN_HORIZONTAL_LEFT,
    LXW_ALIGN_HORIZONTAL_CENTER,
    LXW_ALIGN_HORIZONTAL_RIGHT,
    LXW_ALIGN_HORIZONTAL_FILL,
    LXW_ALIGN_HORIZONTAL_JUSTIFY,
    LXW_ALIGN_HORIZONTAL_CENTER_ACROSS,
    LXW_ALIGN_HORIZONTAL_DISTRIBUTED,
    LXW_ALIGN_HORIZONTAL_LEFT_SHIFTED
} lxw_horizontal_aligns;

typedef enum lxw_fold_styles {
    LXW_FOLD_STYLES_NONE,
    LXW_FOLD_STYLES
} lxw_fold_styles;

typedef enum lxw_sheet_types {
    LXW_SHEET_TYPE_WORKSHEET,
    LXW_SHEET_TYPE_CHART,
    LXW_SHEET_TYPE_VB_PROJECT
} lxw_sheet_types;

typedef enum lxw_filter_modes {
    LXW_FILTER_MODE_AUTOMATIC,
    LXW_FILTER_MODE_MANUAL
} lxw_filter_modes;

typedef enum lxw_page_orders {
    LXW_DOWN_THEN_RIGHT,
    LXW_RIGHT_THEN_DOWN
} lxw_page_orders;

typedef enum lxw_page_orientations {
    LXW_PORTRAIT,
    LXW_LANDSCAPE
} lxw_page_orientations;

typedef enum lxw_page_start_pages {
    LXW_NO_START_PAGE = 0,
    LXW_START_PAGE_1 = 1
} lxw_page_start_pages;

typedef enum lxw_paper_types {
    LXW_PAPER_LETTER,
    LXW_PAPER_LETTER_SMALL,
    LXW_PAPER_TABLOID,
    LXW_PAPER_LEDGER,
    LXW_PAPER_LEGAL,
    LXW_PAPER_STATEMENT,
    LXW_PAPER_EXECUTIVE,
    LXW_PAPER_A3,
    LXW_PAPER_A4,
    LXW_PAPER_A4_SMALL,
    LXW_PAPER_A5,
    LXW_PAPER_B4,
    LXW_PAPER_B5,
    LXW_PAPER_FOLIO,
    LXW_PAPER_QUARTO,
    LXW_PAPER_STANDARD,
    LXW_PAPER_PC_10x14
} lxw_paper_types;

typedef enum lxw_print_options {
    LXW_PRINT_HORIZONTAL_CENTERED = 1,
    LXW_PRINT_VERTICAL_CENTERED = 2,
    LXW_PRINT_MONochrome = 4,
    LXW_PRINT_DRAFT = 8,
    LXW_PRINT_PAGE_ORDER_REVERSE = 16
} lxw_print_options;

typedef enum lxw_builtin_formats {
    LXW_FORMAT_GENERAL = 0,
    LXW_FORMAT_NUMBER = 1,
    LXW_FORMAT_NUMBER_SEP = 2,
    LXW_FORMAT_NUMBER_SEP_PSP = 3,
    LXW_FORMAT_NUMBER_SEP_DPP_PSP = 4,
    LXW_FORMAT_CURRENCY_SEP = 5,
    LXW_FORMAT_CURRENCY_SEP_NEG = 6,
    LXW_FORMAT_CURRENCY_SEP_D_NEG = 7,
    LXW_FORMAT_CURRENCY_SEP_D = 8,
    LXW_FORMAT_PERCENT = 9,
    LXW_FORMAT_PERCENT_2 = 10,
    LXW_FORMAT_SCIENTIFIC = 11,
    LXW_FORMAT_FRACTION_1 = 12,
    LXW_FORMAT_FRACTION_2 = 13,
    LXW_FORMAT_DATE_MDY = 14,
    LXW_FORMAT_DATE_MDY2 = 15,
    LXW_FORMAT_DATE_MMM_YY = 16,
    LXW_FORMAT_CUSTOM_D_MON_YY = 17,
    LXW_FORMAT_CUSTOM_D_MON = 18,
    LXW_FORMAT_CUSTOM_MON_YY = 19,
    LXW_FORMAT_CUSTOM_HMM_AMPM = 20,
    LXW_FORMAT_CUSTOM_HMM = 21,
    LXW_FORMAT_CUSTOM_MMSS = 22,
    LXW_FORMAT_CUSTOM_MMSS_AMPM = 23,
    LXW_FORMAT_CUSTOM_HHMM = 24,
    LXW_FORMAT_CUSTOM_HHMMSS = 25,
    LXW_FORMAT_CUSTOM_DD_MM_YY = 26,
    LXW_FORMAT_CUSTOM_DD_MM_YYYY = 27,
    LXW_FORMAT_CUSTOM_MM_YYYY = 28,
    LXW_FORMAT_NUMBER_SEP_00 = 29,
    LXW_FORMAT_NUMBER_SEP_000 = 30,
    LXW_FORMAT_FRACTION_3 = 31,
    LXW_FORMAT_SCIENTIFIC_D_00 = 32,
    LXW_FORMAT_SCIENTIFIC_D_00_SUPER = 33,
    LXW_FORMAT_TEXT = 34
} lxw_builtin_formats;

typedef enum lxw_tab_colors {
    LXW_TAB_COLOR_NONE,
    LXW_TAB_COLOR_RED,
    LXW_TAB_COLOR_ORANGE,
    LXW_TAB_COLOR_YELLOW,
    LXW_TAB_COLOR_GREEN,
    LXW_TAB_COLOR_BLUE,
    LXW_TAB_COLOR_PURPLE,
    LXW_TAB_COLOR_PINK,
    LXW_TAB_COLOR_TURQUOISE,
    LXW_TAB_COLOR_GRAY_25,
    LXW_TAB_COLOR_GRAY_50,
    LXW_TAB_COLOR_AUTOMATIC
} lxw_tab_colors;

typedef enum lxw_error_types {
    LXW_ERROR_TYPE_NULL = 0x00,
    LXW_ERROR_TYPE_DIV_0 = 0x07,
    LXW_ERROR_TYPE_ERROR = 0x0F,
    LXW_ERROR_TYPE_NAME = 0x17,
    LXW_ERROR_TYPE_REF = 0x17,
    LXW_ERROR_TYPE_NUM = 0x24,
    LXW_ERROR_TYPE_NA = 0x2A,
    LXW_ERROR_TYPE_GETTING_DATA = 0x2A
} lxw_error_types;

typedef enum lxw_chart_error_types {
    LXW_CHART_ERROR_TYPE_FIXED,
    LXW_CHART_ERROR_TYPE_PERCENTAGE,
    LXW_CHART_ERROR_TYPE_STANDARD_DEV,
    LXW_CHART_ERROR_TYPE_STANDARD_DEV_P
} lxw_chart_error_types;

typedef enum lxw_chart_marker_types {
    LXW_CHART_MARKER_NONE,
    LXW_CHART_MARKER_SQUARE,
    LXW_CHART_MARKER_DIAMOND,
    LXW_CHART_MARKER_TRIANGLE,
    LXW_CHART_MARKER_X,
    LXW_CHART_MARKER_STAR,
    LXW_CHART_MARKER_DOT,
    LXW_CHART_MARKER_DOT_SMALL,
    LXW_CHART_MARKER_CIRCLE,
    LXW_CHART_MARKER_PLUS,
    LXW_CHART_MARKER_AUTO
} lxw_chart_marker_types;

typedef enum lxw_chart_axis_labels {
    LXW_CHART_AXIS_LABELS_NONE,
    LXW_CHART_AXIS_LABELS_SINGLE_LEVEL,
    LXW_CHART_AXIS_LABELS_SHOW_SERIES_NAME,
    LXW_CHART_AXIS_LABELS_SHOW_CATEGORY_NAME,
    LXW_CHART_AXIS_LABELS_SHOW_CATEGORY_NAME_AND_SERIES_NAME
} lxw_chart_axis_labels;

typedef enum lxw_chart_axis_tick_mark {
    LXW_CHART_AXIS_TICK_MARK_NONE,
    LXW_CHART_AXIS_TICK_MARK_IN,
    LXW_CHART_AXIS_TICK_MARK_OUT,
    LXW_CHART_AXIS_TICK_MARK_CROSSING
} lxw_chart_axis_tick_mark;

typedef enum lxw_chart_axis_display_unit {
    LXW_CHART_AXIS_DISPLAY_UNIT_NONE,
    LXW_CHART_AXIS_DISPLAY_UNIT_HUNDREDS,
    LXW_CHART_AXIS_DISPLAY_UNIT_THOUSANDS,
    LXW_CHART_AXIS_DISPLAY_UNIT_TEN_THOUSANDS,
    LXW_CHART_AXIS_DISPLAY_UNIT_HUNDRED_THOUSANDS,
    LXW_CHART_AXIS_DISPLAY_UNIT_MILLIONS,
    LXW_CHART_AXIS_DISPLAY_UNIT_TEN_MILLIONS,
    LXW_CHART_AXIS_DISPLAY_UNIT_HUNDRED_MILLIONS,
    LXW_CHART_AXIS_DISPLAY_UNIT_BILLIONS,
    LXW_CHART_AXIS_DISPLAY_UNIT_TRILLIONS
} lxw_chart_axis_display_unit;

typedef enum lxw_chart_label_position {
    LXW_CHART_LABEL_POSITION_DEFAULT,
    LXW_CHART_LABEL_POSITION_CENTER,
    LXW_CHART_LABEL_POSITION_ABOVE,
    LXW_CHART_LABEL_POSITION_RIGHT,
    LXW_CHART_LABEL_POSITION_BELOW,
    LXW_CHART_LABEL_POSITION_LEFT,
    LXW_CHART_LABEL_POSITION_AUTO
} lxw_chart_label_position;

typedef enum lxw_chart_labels_series {
    LXW_CHART_LABELS_SERIES_NAME,
    LXW_CHART_LABELS_CATEGORY_NAME,
    LXW_CHART_LABELS_VALUE,
    LXW_CHART_LABELS_BUBBLE_SIZE,
    LXW_CHART_LABELS_PERCENTAGE,
    LXW_CHART_LABELS_SERIES_CATEGORY,
    LXW_CHART_LABELS_SEPARATOR
} lxw_chart_labels_series;

typedef enum lxw_chart_legend_position {
    LXW_CHART_LEGEND_NONE,
    LXW_CHART_LEGEND_RIGHT,
    LXW_CHART_LEGEND_LEFT,
    LXW_CHART_LEGEND_BOTTOM,
    LXW_CHART_LEGEND_TOP,
    LXW_CHART_LEGEND_TOP_RIGHT
} lxw_chart_legend_position;

typedef enum lxw_chart_datalabel分手 {
    LXW_CHART_DATALABEL分手,
    LXW_CHART_DATALABEL_XX,
    LXW_CHART_DATALABEL_XX_PERCENT,
    LXW_CHART_DATALABEL_XX_CUM_XX
} lxw_chart_datalabel分手;

typedef enum lxw_chart_bar_sales {
    LXW_CHART_BAR_SALES_CLUSTERED,
    LXW_CHART_BAR_SALES_STACKED,
    LXW_CHART_BAR_SALES_PERCENT_STACKED
} lxw_chart_bar_sales;

typedef enum lxw_chart_bar_direction {
    LXW_CHART_BAR_HORIZONTAL,
    LXW_CHART_BAR_VERTICAL
} lxw_chart_bar_direction;

typedef enum lxw_chart_bar_grouping {
    LXW_CHART_BAR_GROUPING_CLUSTERED,
    LXW_CHART_BAR_GROUPING_STACKED,
    LXW_CHART_BAR_GROUPING_PERCENT_STACKED
} lxw_chart_bar_grouping;

typedef enum lxw_chart_pie_types {
    LXW_CHART_PIE,
    LXW_CHART_PIE_EXPLODED,
    LXW_CHART_DOUGHNUT,
    LXW_CHART_DOUGHNUT_EXPLODED
} lxw_chart_pie_types;

typedef enum lxw_chart_scatter_styles {
    LXW_CHART_SCATTER_NONE,
    LXW_CHART_SCATTER_STRAIGHT,
    LXW_CHART_SCATTER_STRAIGHT_WITH_MARKERS,
    LXW_CHART_SCATTER_SMOOTH,
    LXW_CHART_SCATTER_SMOOTH_WITH_MARKERS,
    LXW_CHART_SCATTER_STRAIGHT_DROP_LINES,
    LXW_CHART_SCATTER_SMOOTH_DROP_LINES
} lxw_chart_scatter_styles;

typedef enum lxw_chart_smooth_types {
    LXW_CHART_SMOOTH_NONE,
    LXW_CHART_SMOOTH
} lxw_chart_smooth_types;

typedef enum lxw_chart_line_styles {
    LXW_CHART_LINE_NONE,
    LXW_CHART_LINE_SOLID,
    LXW_CHART_LINE_DASH,
    LXW_CHART_LINE_DOT,
    LXW_CHART_LINE_DASH_DOT,
    LXW_CHART_LINE_LONG_DASH,
    LXW_CHART_LINE_LONG_DASH_DOT,
    LXW_CHART_LINE_LONG_DASH_DOT_DOT
} lxw_chart_line_styles;

typedef enum lxw_chart_markline_types {
    LXW_CHART_MARKLINE_TRANSPARENT,
    LXW_CHART_MARKLINE_SOLID,
    LXW_CHART_MARKLINE_DASH,
    LXW_CHART_MARKLINE_DOT,
    LXW_CHART_MARKLINE_DASH_DOT,
    LXW_CHART_MARKLINE_LONG_DASH,
    LXW_CHART_MARKLINE_LONG_DASH_DOT,
    LXW_CHART_MARKLINE_LONG_DASH_DOT_DOT
} lxw_chart_markline_types;

typedef enum lxw_drawing_aligns {
    LXW_DRAWING_ALIGN_HORIZONTAL,
    LXW_DRAWING_ALIGN_CENTER,
    LXW_DRAWING_ALIGN_RIGHT
} lxw_drawing_aligns;

typedef enum lxw_drawing_valigns {
    LXW_DRAWING_VALIGN_TOP,
    LXW_DRAWING_VALIGN_CENTER,
    LXW_DRAWING_VALIGN_BOTTOM
} lxw_drawing_valigns;

typedef enum lxw_conditional_format_types {
    LXW_CONDITIONAL_TYPE_NONE,
    LXW_CONDITIONAL_CELL_IS,
    LXW_CONDITIONAL_TYPE_AVERAGE,
    LXW_CONDITIONAL_TYPE_COLORS,
    LXW_CONDITIONAL_TYPE_DATA_BAR,
    LXW_CONDITIONAL_TYPE_DUPLICATE_VALUES,
    LXW_CONDITIONAL_TYPE_ERROR,
    LXW_CONDITIONAL_TYPE_EXPRESSION,
    LXW_CONDITIONAL_TYPE_ICON_SETS,
    LXW_CONDITIONAL_TYPE_TOP_10,
    LXW_CONDITIONAL_TYPE_UNIQUE_VALUES,
    LXW_CONDITIONAL_TYPE_CONTAINS_TEXT,
    LXW_CONDITIONAL_TYPE_NOT_CONTAINS_TEXT,
    LXW_CONDITIONAL_TYPE_BEGINS_WITH,
    LXW_CONDITIONAL_TYPE_ENDS_WITH
} lxw_conditional_format_types;

typedef enum lxw_conditional_average_types {
    LXW_CONDITIONAL_AVERAGE_NONE,
    LXW_CONDITIONAL_AVERAGE_ABOVE,
    LXW_CONDITIONAL_AVERAGE_BELOW,
    LXW_CONDITIONAL_AVERAGE_ABOVE_OR_EQUAL,
    LXW_CONDITIONAL_AVERAGE_BELOW_OR_EQUAL,
    LXW_CONDITIONAL_AVERAGE_1_STD_DEV_ABOVE,
    LXW_CONDITIONAL_AVERAGE_1_STD_DEV_BELOW,
    LXW_CONDITIONAL_AVERAGE_2_STD_DEV_ABOVE,
    LXW_CONDITIONAL_AVERAGE_2_STD_DEV_BELOW,
    LXW_CONDITIONAL_AVERAGE_3_STD_DEV_ABOVE,
    LXW_CONDITIONAL_AVERAGE_3_STD_DEV_BELOW
} lxw_conditional_average_types;

typedef enum lxw_conditional_rule_types {
    LXW_CONDITIONAL_RULE_NONE,
    LXW_CONDITIONAL_RULE_TYPE_SET,
    LXW_CONDITIONAL_RULE_TYPE_AVERAGE,
    LXW_CONDITIONAL_RULE_TYPE_TOP_10,
    LXW_CONDITIONAL_RULE_TYPE_SPECIAL,
    LXW_CONDITIONAL_RULE_TYPE_CELL_IS,
    LXW_CONDITIONAL_RULE_TYPE_FORMULA,
    LXW_CONDITIONAL_RULE_TYPE_CONTAINS_TEXT,
    LXW_CONDITIONAL_RULE_TYPE_NOT_CONTAINS_TEXT,
    LXW_CONDITIONAL_RULE_TYPE_BEGINS_WITH,
    LXW_CONDITIONAL_RULE_TYPE_ENDS_WITH
} lxw_conditional_rule_types;

typedef enum lxw_conditional_text_types {
    LXW_CONDITIONAL_TEXT_NONE,
    LXW_CONDITIONAL_TEXT_SPECIAL,
    LXW_CONDITIONAL_TEXT_BEGINS_WITH,
    LXW_CONDITIONAL_TEXT_ENDS_WITH,
    LXW_CONDITIONAL_TEXT_CONTAINS,
    LXW_CONDITIONAL_TEXT_NOT_CONTAINS
} lxw_conditional_text_types;

typedef enum lxw_conditional_dxf_types {
    LXW_CONDITIONAL_DXF_TYPE_DELETE,
    LXW_CONDITIONAL_DXF_TYPE_FORMULA,
    LXW_CONDITIONAL_DXF_TYPE_CONDITIONAL_FORMATTING
} lxw_conditional_dxf_types;

typedef enum lxw_conditional_top_types {
    LXW_CONDITIONAL_TOP_10,
    LXW_CONDITIONAL_BOTTOM_10,
    LXW_CONDITIONAL_TOP_10_PERCENT,
    LXW_CONDITIONAL_BOTTOM_10_PERCENT
} lxw_conditional_top_types;

typedef enum lxw_conditional_icon_set_types {
    LXW_CONDITIONAL_ICON_3_ARROWS,
    LXW_CONDITIONAL_ICON_3_ARROWS_GRAY,
    LXW_CONDITIONAL_ICON_3_FLAGS,
    LXW_CONDITIONAL_ICON_3_TRAFFIC_LIGHTS_1,
    LXW_CONDITIONAL_ICON_3_TRAFFIC_LIGHTS_2,
    LXW_CONDITIONAL_ICON_3_SIGNS,
    LXW_CONDITIONAL_ICON_3_SYMBOLS,
    LXW_CONDITIONAL_ICON_3_SYMBOLS_2,
    LXW_CONDITIONAL_ICON_4_ARROWS,
    LXW_CONDITIONAL_ICON_4_ARROWS_GRAY,
    LXW_CONDITIONAL_ICON_4_RED_TO_BLACK,
    LXW_CONDITIONAL_ICON_4_CIRCLES,
    LXW_CONDITIONAL_ICON_5_ARROWS,
    LXW_CONDITIONAL_ICON_5_ARROWS_GRAY,
    LXW_CONDITIONAL_ICON_5_RATINGS
} lxw_conditional_icon_set_types;

typedef enum lxw_conditional_bar_sales {
    LXW_CONDITIONAL_BAR_SALES_BAR_SALES,
    LXW_CONDITIONAL_BAR_SALES_BAR_GAP,
    LXW_CONDITIONAL_BAR_SALES_PERCENT_GAP
} lxw_conditional_bar_sales;

typedef enum lxw_conditional_bar_dir {
    LXW_CONDITIONAL_BAR_DIR_LR,
    LXW_CONDITIONAL_BAR_DIR_TB
} lxw_conditional_bar_dir;

typedef enum lxw_data_validate_types {
    LXW_VALIDATION_TYPE_NONE,
    LXW_VALIDATION_TYPE_WHOLE,
    LXW_VALIDATION_TYPE_DECIMAL,
    LXW_VALIDATION_TYPE_LIST,
    LXW_VALIDATION_TYPE_DATE,
    LXW_VALIDATION_TYPE_TIME,
    LXW_VALIDATION_TYPE_TEXT_LENGTH,
    LXW_VALIDATION_TYPE_CUSTOM
} lxw_data_validate_types;

typedef enum lxw_data_validated_operation {
    LXW_VALIDATION_OP_NONE,
    LXW_VALIDATION_OP_BETWEEN,
    LXW_VALIDATION_OP_NOT_BETWEEN,
    LXW_VALIDATION_OP_EQUAL,
    LXW_VALIDATION_OP_NOT_EQUAL,
    LXW_VALIDATION_OP_LESS_THAN,
    LXW_VALIDATION_OP_LESS_THAN_OR_EQUAL,
    LXW_VALIDATION_OP_GREATER_THAN,
    LXW_VALIDATION_OP_GREATER_THAN_OR_EQUAL
} lxw_data_validated_operation;

typedef enum lxw_data_validation_error_types {
    LXW_VALIDATION_ERROR_TYPE_STOP,
    LXW_VALIDATION_ERROR_TYPE_WARNING,
    LXW_VALIDATION_ERROR_TYPE_INFORMATION
} lxw_data_validation_error_types;

typedef enum lxw_data_validation_ijm {
    LXW_VALIDATION_IJM_IGNORE,
    LXW_VALIDATION_IJM_SHRINK_TO_FIT,
    LXW_VALIDATION_IJM_WRAP_TEXT
} lxw_data_validation_ijm;

typedef enum lxw_data_validation_mv {
    LXW_VALIDATION_MV_SHOW_TITLE,
    LXW_VALIDATION_MV_NOT_SHOW_TITLE,
    LXW_VALIDATION_MV_FOCUS,
    LXW_VALIDATION_MV_NOT_FOCUS,
    LXW_VALIDATION_MV_SELECT,
    LXW_VALIDATION_MV_NOT_SELECT,
    LXW_VALIDATION_MV_EDIT,
    LXW_VALIDATION_MV_NOT_EDIT
} lxw_data_validation_mv;

typedef enum lxw_print_scale_types {
    LXW_PRINT_SCALE_AUTO,
    LXW_PRINT_SCALE_PAGE_WIDTH,
    LXW_PRINT_SCALE_FIT_PAGES
} lxw_print_scale_types;

typedef enum lxw_autofilter_op {
    LXW_AUTOFILTER_OP_AND,
    LXW_AUTOFILTER_OP_OR
} lxw_autofilter_op;

typedef enum lxw_autofilter_filter {
    LXW_AUTOFILTER_FILTER_NONE,
    LXW_AUTOFILTER_FILTER_COUNTS,
    LXW_AUTOFILTER_FILTER_CELLS,
    LXW_AUTOFILTER_FILTER_SELECTED
} lxw_autofilter_filter;

typedef enum lxw_match_rules {
    LXW_MATCH_RULE_IMPORTANT,
    LXW_MATCH_RULE_NORMAL,
    LXW_MATCH_RULE_ROW_BAND,
    LXW_MATCH_RULE_COL_BAND,
    LXW_MATCH_RULE_PAGE_AREA,
    LXW_MATCH_RULE_PAGE_FIELD,
    LXW_MATCH_RULE_PAGE_ITEM,
    LXW_MATCH_RULE_SHEET_GLOBAL,
    LXW_MATCH_RULE_SHEET_LOCAL,
    LXW_MATCH_RULE_TABLE_HEADER,
    LXW_MATCH_RULE_TABLE_DATA,
    LXW_MATCH_RIAL_TABLE_TOTALS,
    LXW_MATCH_RULE_COLUMN_TOTALS,
    LXW_MATCH_RULE_ROW_TOTALS
} lxw_match_rules;

typedef enum lxw_protection_types {
    LXW_PROTECTION_NONE,
    LXW_PROTECTION_HIDDEN,
    LXW_PROTECTION_LOCKED,
    LXW_PROTECTION_LOCKED_AND_HIDDEN
} lxw_protection_types;

typedef enum lxw_sparklines {
    LXW_SPARKLINE_HORIZONTAL,
    LXW_SPARKLINE_VERTICAL
} lxw_sparklines;

typedef enum lxw_sparkline_types {
    LXW_SPARKLINE_TYPE_LINE,
    LXW_SPARKLINE_TYPE_COLUMN,
    LXW_SPARKLINE_TYPE_STANDARD
} lxw_sparkline_types;

typedef enum lxw_sparkline_markers {
    LXW_SPARKLINE_NO_MARKERS,
    LXW_SPARKLINE_MARKERS
} lxw_sparkline_markers;

typedef enum lxw_sparkline_axis_types {
    LXW_SPARKLINE_AXIS_TYPE_SPREAD,
    LXW_SPARKLINE_AXIS_TYPE_CENTER,
    LXW_SPARKLINE_AXIS_TYPE_27_AXIS
} lxw_sparkline_axis_types;

typedef enum lxw_sparkline_tags {
    LXW_SPARKLINE_NO_TAGS,
    LXW_SPARKLINE_TAGS
} lxw_sparkline_tags;

typedef enum lxw_sparkline_range_types {
    LXW_SPARKLINE_RANGE_TYPE_SINGLE_CELL,
    LXW_SPARKLINE_RANGE_TYPE_MULTI_CELL
} lxw_sparkline_range_types;

typedef enum lxw_worksheet_type {
    LXW_WORKBOOK_TYPE_WORKSHEET,
    LXW_WORKBOOK_TYPE_CHART
} lxw_worksheet_type;

typedef enum lxw_worksheet_orien {
    LXW_WORKBOOK_ORIEN_PORTRAIT,
    LXW_WORKBOOK_ORIEN_LANDSCAPE
} lxw_worksheet_orien;

typedef enum lxw_worksheet_scope {
    LXW_WORKBOOK_SCOPE_SHEET,
    LXW_WORKBOOK_SCOPE_WORKBOOK
} lxw_worksheet_scope;

typedef enum lxw_defined_value_func {
    LXW_DEFINED_VALUE_FUNC_NONE,
    LXW_DEFINED_VALUE_FUNC_CONCAT,
    LXW_DEFINED_VALUE_FUNC_TILDE,
    LXW_DEFINED_VALUE_FUNC_SUM
} lxw_defined_value_func;

struct lxw_border {
    uint8_t left_style;
    uint8_t right_style;
    uint8_t top_style;
    uint8_t bottom_style;
    uint32_t left_color;
    uint32_t right_color;
    uint32_t top_color;
    uint32_t bottom_color;
    uint8_t diagonal_up;
    uint8_t diagonal_down;
    uint8_t diagonal_color_type;
    uint32_t diagonal_color;
};

struct lxw_fill {
    uint8_t type;
    uint8_t pattern;
    uint8_t fg_color_type;
    uint8_t bg_color_type;
    uint32_t fg_color;
    uint32_t bg_color;
};

struct lxw_font {
    uint8_t bold;
    uint8_t italic;
    uint8_t underline;
    uint8_t charset;
    uint8_t family;
    uint8_t scheme;
    uint8_t color_type;
    uint32_t color;
    uint8_t size;
    uint8_t name_length;
    char name[64];
};

struct lxw_format {
    uint8_t bold;
    uint8_t italic;
    uint8_t underline;
    uint8_t font_charset;
    uint8_t font_family;
    uint8_t font_scheme;
    uint32_t font_color;
    uint16_t font_size;
    char font_name[64];
    uint16_t num_format;
    uint16_t num_format_index;
    uint8_t align_h;
    uint8_t align_v;
    uint8_t text_wrap;
    uint8_t justify;
    uint8_t align_indent;
    uint8_t shrink;
    uint8_t text_justlast;
    uint32_t bg_color;
    uint32_t fg_color;
    uint8_t pattern;
    uint8_t border_index;
    uint8_t border_left_style;
    uint8_t border_right_style;
    uint8_t border_top_style;
    uint8_t border_bottom_style;
    uint32_t border_color;
    uint8_t border_index_diag;
    uint8_t border_diag_style;
    uint32_t border_diag_color;
    uint8_t hidden;
    uint8_t locked;
};

lxw_format* workbook_add_format(lxw_workbook* workbook);
void format_set_bold(lxw_format* format);
void format_set_bg_color(lxw_format* format, uint32_t color);
void format_set_font_color(lxw_format* format, uint32_t color);
void format_set_border(lxw_format* format, uint8_t style);

lxw_workbook* workbook_new(const char* filename);
lxw_error workbook_close(lxw_workbook* workbook);
lxw_worksheet* workbook_add_worksheet(lxw_workbook* workbook, const char* sheetname);
lxw_format* workbook_add_format(lxw_workbook* workbook);

void worksheet_set_column(worksheet, int first_col, int last_col, double width, lxw_format* format);
lxw_error worksheet_write_string(lxw_worksheet* worksheet, int row, int col, const char* string, lxw_format* format);
lxw_error worksheet_write_number(lxw_worksheet* worksheet, int row, int col, double number, lxw_format* format);

#ifdef __cplusplus
}
#endif

#endif