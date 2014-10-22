
#include "tooltip_manager.h"

#include <vector>

#include "base/string_split.h"

#include "ui_gfx/font.h"

#include "ui_base/text/text_elider.h"

namespace
{
    
    // Maximum number of characters we allow in a tooltip.
    static const size_t kMaxTooltipLength = 1024;

    // Maximum number of lines we allow in the tooltip.
    static const size_t kMaxLines = 6;

}

namespace view
{

    // static
    void TooltipManager::TrimTooltipToFit(string16* text,
        int* max_width,
        int* line_count,
        int x,
        int y)
    {
        *max_width = 0;
        *line_count = 0;

        // Clamp the tooltip length to kMaxTooltipLength so that we don't
        // accidentally DOS the user with a mega tooltip.
        if(text->length() > kMaxTooltipLength)
        {
            *text = text->substr(0, kMaxTooltipLength);
        }

        // Determine the available width for the tooltip.
        int available_width = GetMaxWidth(x, y);

        // Split the string into at most kMaxLines lines.
        std::vector<string16> lines;
        base::SplitString(*text, '\n', &lines);
        if(lines.size() > kMaxLines)
        {
            lines.resize(kMaxLines);
        }
        *line_count = static_cast<int>(lines.size());

        // Format each line to fit.
        gfx::Font font = GetDefaultFont();
        string16 result;
        for(std::vector<string16>::iterator i=lines.begin(); i!=lines.end(); ++i)
        {
            string16 elided_text = ui::ElideText(*i, font, available_width, false);
            *max_width = std::max(*max_width, font.GetStringWidth(elided_text));
            if(!result.empty())
            {
                result.push_back('\n');
            }
            result.append(elided_text);
        }
        *text = result;
    }

} //namespace view