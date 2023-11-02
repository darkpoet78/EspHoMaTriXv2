#include "esphome.h"

namespace esphome
{

#ifdef USE_Fireplugin
  const size_t m_heatSize = 8 * 32; /**< Number of heat temperatures */
  uint8_t *m_heat = new (std::nothrow) uint8_t[m_heatSize];
  /**
   * Cooling: How much does the air cool as it rises?
   * Less cooling => taller flames.
   * More cooling => shorter flames.
   */
  static const uint8_t COOLING = 60U;

  /**
   * Sparking: What chance (out of 255) is there that a new spark will be lit?
   * Higher chance = more roaring fire.  Lower chance = more flickery fire.
   */
  static const uint8_t SPARKING = 120U;

  /**
   * Approximates a 'black body radiation' spectrum for a given 'heat' level.
   * This is useful for animations of 'fire'.
   * Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
   * This is NOT a chromatically correct 'black body radiation'
   * spectrum, but it's surprisingly close, and it's fast and small.
   */
  Color heatColor(uint8_t temperature);
#endif
#ifdef USE_Fireplugin

  Color EHMTX_queue::heatColor(uint8_t temperature)
  {
    Color heatColor;

    /* Scale 'heat' down from 0-255 to 0-191, which can then be easily divided
     * into three equal 'thirds' of 64 units each.
     */
    uint8_t t192 = static_cast<uint32_t>(temperature) * 191U / 255U;

    /* Calculate a value that ramps up from zero to 255 in each 'third' of the scale. */
    uint8_t heatRamp = t192 & 0x3fU; /* 0..63 */

    /* Scale up to 0..252 */
    heatRamp <<= 2;

    /* Now figure out which third of the spectrum we're in. */
    if (t192 & 0x80U)
    {
      /* We're in the hottest third */
      heatColor = Color(255U, 255U, heatRamp); /* Ramp up blue */
    }
    else if (t192 & 0x40U)
    {
      /* We're in the middle third */
      heatColor = Color(255U, heatRamp, 0U); /* No blue */
    }
    else
    {
      /* We're in the coolest third */
      heatColor = Color(heatRamp, 0, 0);
    }

    return heatColor;
  }

#endif

  EHMTX_queue::EHMTX_queue(EHMTX *config)
  {
    this->config_ = config;
    this->endtime = 0;
    this->last_time = 0;
    this->screen_time_ = 0;
    this->mode = MODE_EMPTY;
    this->icon_name = "";
    this->icon = 0;
    this->text = "";
    this->default_font = true;
    this->progress = 0;
    this->sbitmap = nullptr;
    this->progressbar_color = esphome::display::COLOR_OFF;
    this->progressbar_back_color = esphome::display::COLOR_OFF;
  }

  void EHMTX_queue::status()
  {
    switch (this->mode)
    {
    case MODE_EMPTY:
      ESP_LOGD(TAG, "empty slot");
      break;
    case MODE_BLANK:
      ESP_LOGD(TAG, "queue: blank screen for %d sec", this->screen_time_);
      break;
    case MODE_COLOR:
      ESP_LOGD(TAG, "queue: color screen for %d sec", this->screen_time_);
      break;
    case MODE_CLOCK:
      ESP_LOGD(TAG, "queue: clock for: %d sec", this->screen_time_);
      break;
    case MODE_DATE:
      ESP_LOGD(TAG, "queue: date for: %d sec", this->screen_time_);
      break;
    case MODE_GRAPH_SCREEN:
      ESP_LOGD(TAG, "queue: graph for: %d sec", this->screen_time_);
      break;
    case MODE_FULL_SCREEN:
      ESP_LOGD(TAG, "queue: full screen: \"%s\" for: %d sec", this->icon_name.c_str(), this->screen_time_);
      break;
    case MODE_ICON_SCREEN:
      ESP_LOGD(TAG, "queue: icon screen: \"%s\" text: %s for: %d sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_);
      break;
    case MODE_ICON_PROGRESS:
      ESP_LOGD(TAG, "queue: icon progress: \"%s\" text: %s for: %d sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_);
      break;
    case MODE_ICON_CLOCK:
      ESP_LOGD(TAG, "queue: icon clock: \"%s\" for: %d sec", this->icon_name.c_str(), this->screen_time_);
      break;
    case MODE_ICON_DATE:
      ESP_LOGD(TAG, "queue: icon date: \"%s\" for: %d sec", this->icon_name.c_str(), this->screen_time_);
      break;
    case MODE_ALERT_SCREEN:
      ESP_LOGD(TAG, "queue: icon: \"%s\" for: %d sec", this->icon_name.c_str(), this->screen_time_);
      break;
    case MODE_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: text text: \"%s\" for: %d sec", this->text.c_str(), this->screen_time_);
      break;
    case MODE_RAINBOW_ICON:
      ESP_LOGD(TAG, "queue: rainbow icon: \"%s\" text: %s for: %d sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_);
      break;
    case MODE_RAINBOW_TEXT:
      ESP_LOGD(TAG, "queue: rainbow text: \"%s\" for: %d sec", this->text.c_str(), this->screen_time_);
      break;
    case MODE_RAINBOW_CLOCK:
      ESP_LOGD(TAG, "queue: rainbow clock for: %d sec", this->screen_time_);
      break;
    case MODE_RAINBOW_DATE:
      ESP_LOGD(TAG, "queue: rainbow date for: %d sec", this->screen_time_);
      break;
    case MODE_ICON_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: icon text screen: \"%s\" text: %s for: %d sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_);
      break;
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      ESP_LOGD(TAG, "queue: rainbow icon text screen: \"%s\" text: %s for: %d sec", this->icon_name.c_str(), this->text.c_str(), this->screen_time_);
      break;
    case MODE_FIRE:
      ESP_LOGD(TAG, "queue: fire for: %d sec", this->screen_time_);
      break;

#ifndef USE_ESP8266
    case MODE_BITMAP_SCREEN:
      ESP_LOGD(TAG, "queue: bitmap for: %d sec", this->screen_time_);
      break;
    case MODE_BITMAP_SMALL:
      ESP_LOGD(TAG, "queue: small bitmap for: %d sec", this->screen_time_);
      break;
    case MODE_RAINBOW_BITMAP_SMALL:
      ESP_LOGD(TAG, "queue: rainbow small bitmap for: %d sec", this->screen_time_);
      break;
#endif

    default:
      ESP_LOGD(TAG, "queue: UPPS");
      break;
    }
  }

  int EHMTX_queue::xpos()
  {
    uint8_t width = 32;
    uint8_t startx = 0;
    int result = 0;
    switch (this->mode)
    {
    case MODE_RAINBOW_ICON:
    case MODE_BITMAP_SMALL:
    case MODE_RAINBOW_BITMAP_SMALL:
    case MODE_ICON_SCREEN:
    case MODE_ICON_CLOCK:
    case MODE_ICON_DATE:
    case MODE_ALERT_SCREEN:
    case MODE_ICON_PROGRESS:
      startx = 8;
      break;
    case MODE_TEXT_SCREEN:
    case MODE_RAINBOW_TEXT:
      // no correction
      break;
    case MODE_ICON_TEXT_SCREEN:
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      if (this->pixels_ < 32)
      {
        startx = 8;
      }
      break;
    default:
      break;
    }

    if (this->config_->display_gauge)
    {
      startx += 2;
    }
    width -= startx;

#ifdef EHMTXv2_USE_RTL
    if (this->pixels_ < width)
    {
      result = 32 - ceil((width - this->pixels_) / 2);
    }
    else
    {

      result = startx + this->config_->scroll_step;
    }
#else
#ifdef EHMTXv2_SCROLL_SMALL_TEXT
    result = startx - this->config_->scroll_step + width;
#else
    if (this->pixels_ < width)
    {
      result = startx + ceil((width - this->pixels_) / 2);
    }
    else
    {
      result = startx - this->config_->scroll_step + width;
    }
#endif
#endif
    return result;
  }

  void EHMTX_queue::update_screen()
  {
    if (millis() - this->config_->last_rainbow_time >= EHMTXv2_RAINBOW_INTERVALL)
    {
      this->config_->hue_++;
      if (this->config_->hue_ == 360)
      {
        this->config_->hue_ = 0;
      }
      float red, green, blue;
      esphome::hsv_to_rgb(this->config_->hue_, 0.8, 0.8, red, green, blue);
      this->config_->rainbow_color = Color(uint8_t(255 * red), uint8_t(255 * green), uint8_t(255 * blue));
      this->config_->last_rainbow_time = millis();
    }

    if (this->icon < this->config_->icon_count)
    {
      if (millis() - this->config_->last_anim_time >= this->config_->icons[this->icon]->frame_duration)
      {
        this->config_->icons[this->icon]->next_frame();
        this->config_->last_anim_time = millis();
      }
    }
  }

  void EHMTX_queue::draw()
  {
    display::BaseFont *font = this->default_font ? this->config_->default_font : this->config_->special_font;
    display::BaseFont *info_font = this->config_->info_font ? this->config_->default_font : this->config_->special_font;
    
    int8_t yoffset = this->default_font ? EHMTXv2_DEFAULT_FONT_OFFSET_Y : EHMTXv2_SPECIAL_FONT_OFFSET_Y;
    int8_t xoffset = this->default_font ? EHMTXv2_DEFAULT_FONT_OFFSET_X : EHMTXv2_SPECIAL_FONT_OFFSET_X;

    Color color_;

    if (this->config_->is_running)
    {
      this->config_->display->clear();
      switch (this->mode)
      {
      case MODE_BLANK:
        break;

      case MODE_COLOR:
        this->config_->display->fill(this->text_color);
        break;

      case MODE_BITMAP_SCREEN:
#ifndef USE_ESP8266
        for (uint8_t x = 0; x < 32; x++)
        {
          for (uint8_t y = 0; y < 8; y++)
          {
            this->config_->display->draw_pixel_at(x, y, this->config_->bitmap[x + y * 32]);
          }
        }
#endif
        break;

#ifdef USE_GRAPH
      case MODE_GRAPH_SCREEN:
        if (this->icon == MAXICONS)
        {
          this->config_->display->graph(0, 0, this->config_->graph);
        }
        else
        {
          this->config_->display->graph(8, 0, this->config_->graph);
          if (this->icon != BLANKICON)
          {
            this->config_->display->image(0, 0, this->config_->icons[this->icon]);
          }
        }
        break;
#endif

      case MODE_BITMAP_SMALL:
      case MODE_RAINBOW_BITMAP_SMALL:
#ifndef USE_ESP8266
        color_ = (this->mode == MODE_RAINBOW_BITMAP_SMALL) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
#endif
        if (this->sbitmap != NULL)
        {
          if (this->config_->display_gauge)
          {
            this->config_->display->line(10, 0, 10, 7, esphome::display::COLOR_OFF);
            for (uint8_t x = 0; x < 8; x++)
            {
              for (uint8_t y = 0; y < 8; y++)
              {
                this->config_->display->draw_pixel_at(x + 2, y, this->sbitmap[x + y * 8]);
              }
            }
          }
          else
          {
            this->config_->display->line(8, 0, 8, 7, esphome::display::COLOR_OFF);
            for (uint8_t x = 0; x < 8; x++)
            {
              for (uint8_t y = 0; y < 8; y++)
              {
                this->config_->display->draw_pixel_at(x, y, this->sbitmap[x + y * 8]);
              }
            }
          }
        }
#endif
        break;

      case MODE_RAINBOW_CLOCK:
      case MODE_CLOCK:
        if (this->config_->clock->now().is_valid()) // valid time
        {
          color_ = (this->mode == MODE_RAINBOW_CLOCK) ? this->config_->rainbow_color : this->text_color;
          time_t ts = this->config_->clock->now().timestamp;
          if (this->config_->replace_time_date_active) // check for replace active
          {
            std::string time_new = this->config_->clock->now().strftime(EHMTXv2_TIME_FORMAT).c_str();
            time_new = this->config_->replace_time_date(time_new);
            this->config_->display->printf(xoffset + 15, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
          } else {
            this->config_->display->strftime(xoffset + 15, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_TIME_FORMAT,
                                             this->config_->clock->now());                    
          }
          if ((this->config_->clock->now().second % 2 == 0) && this->config_->show_seconds)
          {
            this->config_->display->draw_pixel_at(0, 0, color_);
          }
          if (this->mode != MODE_RAINBOW_CLOCK)
          {
            this->config_->draw_day_of_week();
          }
        }
        else
        {
          this->config_->display->print(15 + xoffset, yoffset, font, this->config_->alarm_color, display::TextAlign::BASELINE_CENTER, "!t!");
        }
        break;

      case MODE_RAINBOW_DATE:
      case MODE_DATE:
        if (this->config_->clock->now().is_valid())
        {
          color_ = (this->mode == MODE_RAINBOW_DATE) ? this->config_->rainbow_color : this->text_color;
          time_t ts = this->config_->clock->now().timestamp;
          if (this->config_->replace_time_date_active) // check for replace active
          {
            std::string time_new = this->config_->clock->now().strftime(EHMTXv2_DATE_FORMAT).c_str();
            time_new = this->config_->replace_time_date(time_new);
            this->config_->display->printf(xoffset + 15, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
          } else {
            this->config_->display->strftime(xoffset + 15, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_DATE_FORMAT,
                                             this->config_->clock->now());                    
          }
          if ((this->config_->clock->now().second % 2 == 0) && this->config_->show_seconds)
          {
            this->config_->display->draw_pixel_at(0, 0, color_);
          }
          if (this->mode != MODE_RAINBOW_DATE)
          {
            this->config_->draw_day_of_week();
          }
        }
        else
        {
          this->config_->display->print(xoffset + 15, yoffset, font, this->config_->alarm_color, display::TextAlign::BASELINE_CENTER, "!d!");
        }
        break;

      case MODE_FULL_SCREEN:
        this->config_->display->image(0, 0, this->config_->icons[this->icon]);
        break;

      case MODE_ICON_CLOCK:
      case MODE_ICON_DATE:
        if (this->config_->clock->now().is_valid()) // valid time
        {
          color_ = this->text_color;
          time_t ts = this->config_->clock->now().timestamp;
          if (this->mode == MODE_ICON_CLOCK)
          {
            if (this->config_->replace_time_date_active) // check for replace active
            {
              std::string time_new = this->config_->clock->now().strftime(EHMTXv2_TIME_FORMAT).c_str();
              time_new = this->config_->replace_time_date(time_new);
              this->config_->display->printf(xoffset + 19, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
            } else {
              this->config_->display->strftime(xoffset + 19, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_TIME_FORMAT,
                                               this->config_->clock->now());
            }
          }
          else
          {
            if (this->config_->replace_time_date_active) // check for replace active
            {
              std::string time_new = this->config_->clock->now().strftime(EHMTXv2_DATE_FORMAT).c_str();
              time_new = this->config_->replace_time_date(time_new);
              this->config_->display->printf(xoffset + 19, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, "%s", time_new.c_str());
            } else {
              this->config_->display->strftime(xoffset + 19, yoffset, font, color_, display::TextAlign::BASELINE_CENTER, EHMTXv2_DATE_FORMAT,
                                               this->config_->clock->now());
            }
          }
          if (this->icon != BLANKICON)
          {
            this->config_->display->image(0, 0, this->config_->icons[this->icon]);
          }
          this->config_->draw_day_of_week(true);

          if (this->icon_name.find("day") != std::string::npos || this->icon_name.find("weekday") != std::string::npos)
          {
            int mode = 0;
            std::size_t pos = icon_name.find("#");
            if (pos != std::string::npos)
            {
              std::string str_mode = icon_name.substr(pos + 1);
              if (str_mode.length())
              {
                mode = std::stoi(str_mode);
              }
            }

            uint8_t x_left = 0;  // Leftmost point
            uint8_t x_right = 9; // Rightmost point

            if (this->icon_name.rfind("day", 0) == 0)
            {
              uint8_t d = this->config_->clock->now().day_of_month;

              // The symbol consists of a visible part, and an empty area to the right with a width of one point.
              uint8_t l_width = this->config_->GetTextWidth(info_font, "%d", d / 10 % 10);
              uint8_t r_width = this->config_->GetTextWidth(info_font, "%d", d % 10);
              switch (mode)
              {
              // To the center
              case 1:
              // To the center, the left one is a pixel higher.
              case 3:
              // To the center, the right one is a pixel higher.
              case 4:
              // To the center, without leading 0
              case 5:
                x_left = (l_width < 5) ? 5 - l_width : 0;
                x_right = 4;
                break;
              // Left to center, Right to edge
              case 2:
                x_left = (l_width < 5) ? 5 - l_width : 0;
                x_right = x_right - r_width;
                break;
              // To the edges
              default:
                x_right = x_right - r_width;
                break;
              }
              if (mode == 5 && (d < 10))
              {
                x_right = 4 - (r_width - 1) / 2;
                this->config_->display->printf(x_right, yoffset + this->config_->info_y_offset, info_font, this->config_->info_rcolor, display::TextAlign::BASELINE_LEFT, "%d", d % 10);
              }
              else
              {
                this->config_->display->printf(x_left, yoffset + this->config_->info_y_offset - (mode != 3 ? 0 : 1), info_font, this->config_->info_lcolor, display::TextAlign::BASELINE_LEFT, "%d", d / 10 % 10);
                this->config_->display->printf(x_right, yoffset + this->config_->info_y_offset - (mode != 4 ? 0 : 1), info_font, this->config_->info_rcolor, display::TextAlign::BASELINE_LEFT, "%d", d % 10);
              }
            }
            else // if (this->icon_name.rfind("weekday", 0) == 0)
            {
              uint8_t wd = this->config_->clock->now().day_of_week;

              if (this->config_->weekday_char_count > 7)
              {
                std::string left = this->config_->GetWeekdayChar((wd - 1) * 2);
                std::string right = this->config_->GetWeekdayChar((wd - 1) * 2 + 1);

                // The symbol consists of a visible part, and an empty area to the right with a width of one point.
                uint8_t l_width = this->config_->GetTextWidth(info_font, "%s", left.c_str());
                uint8_t r_width = this->config_->GetTextWidth(info_font, "%s", right.c_str());

                switch (mode)
                {
                // To the center
                case 1:
                // To the center, the left one is a pixel higher.
                case 3:
                // To the center, the right one is a pixel higher.
                case 4:
                  x_left = (l_width < 5) ? 5 - l_width : 0;
                  x_right = 4;
                  break;
                // Left to center, Right to edge
                case 2:
                  x_left = (l_width < 5) ? 5 - l_width : 0;
                  x_right = x_right - r_width;
                  break;
                // To the edges
                default:
                  x_right = x_right - r_width;
                  break;
                }
                this->config_->display->printf(x_left, yoffset + this->config_->info_y_offset - (mode != 3 ? 0 : 1), info_font, this->config_->info_lcolor, display::TextAlign::BASELINE_LEFT, "%s", left.c_str());
                this->config_->display->printf(x_right, yoffset + this->config_->info_y_offset - (mode != 4 ? 0 : 1), info_font, this->config_->info_rcolor, display::TextAlign::BASELINE_LEFT, "%s", right.c_str());
              }
              else
              {
                std::string weekday = this->config_->GetWeekdayChar(wd - 1);

                // The symbol consists of a visible part, and an empty area to the right with a width of one point.
                uint8_t c_width = this->config_->GetTextWidth(info_font, "%s", weekday.c_str());
                x_left = 4 - (c_width - 1) / 2;
                this->config_->display->printf(x_left, yoffset + this->config_->info_y_offset, info_font, this->config_->info_lcolor, display::TextAlign::BASELINE_LEFT, "%s", weekday.c_str());
              }
            }
          }
        }
        else
        {
          this->config_->display->print( xoffset + 19, yoffset, font, this->config_->alarm_color, display::TextAlign::BASELINE_CENTER, "!t!");
        }
        break;

      case MODE_ICON_SCREEN:
      case MODE_ALERT_SCREEN:
      case MODE_RAINBOW_ICON:
      case MODE_ICON_PROGRESS:
        color_ = (this->mode == MODE_RAINBOW_ICON) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
#endif
        if (this->mode == MODE_ICON_PROGRESS)
        {
          this->config_->display->line(8, 0, 8, 7, esphome::display::COLOR_OFF);
          if (this->icon != BLANKICON)
          {
            this->config_->display->image(0, 0, this->config_->icons[this->icon]);
          }

          if (this->progress != 0)
          {
            if (this->progressbar_color == esphome::display::COLOR_OFF)
            {
              color_ = esphome::light::ESPHSVColor(this->progress * 120 / 100 + (this->progress < 0 ? 120 : 0), 255, 240).to_rgb();
            }
            else
            {
              color_ = this->progressbar_color;
              this->config_->display->line(9, 7, 31, 7, this->progressbar_back_color);
            }
            this->config_->display->line(9, 7, 9 + abs(this->progress) * 22 / 100, 7, color_);
          }
        }
        else
        {
          if (this->config_->display_gauge)
          {
            if (this->icon != BLANKICON)
            {
              this->config_->display->image(2, 0, this->config_->icons[this->icon]);
            }
            this->config_->display->line(10, 0, 10, 7, esphome::display::COLOR_OFF);
          }
          else
          {
            this->config_->display->line(8, 0, 8, 7, esphome::display::COLOR_OFF);
            if (this->icon != BLANKICON)
            {
              this->config_->display->image(0, 0, this->config_->icons[this->icon]);
            }
          }
        }
        break;

      case MODE_ICON_TEXT_SCREEN:
      case MODE_RAINBOW_ICON_TEXT_SCREEN:
        color_ = (this->mode == MODE_RAINBOW_TEXT) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
#endif
        if (this->icon != BLANKICON)
        {
          int x = 0;
          if (this->pixels_ > 23)
          {
            if (this->xpos() > 23)
            {
              x = 24 - this->xpos();
            }
            else
            {
              if (this->xpos() < 9)
              {
                x = this->xpos() - 9;
              }
            }
          }
          this->config_->display->line(x + 8, 0, x + 8, 7, esphome::display::COLOR_OFF);
          this->config_->display->image(x, 0, this->config_->icons[this->icon]);
        }
        break;

      case MODE_TEXT_SCREEN:
      case MODE_RAINBOW_TEXT:
        color_ = (this->mode == MODE_RAINBOW_TEXT) ? this->config_->rainbow_color : this->text_color;
#ifdef EHMTXv2_USE_RTL
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_RIGHT,
                                      this->text.c_str());
#else
        this->config_->display->print(this->xpos() + xoffset, yoffset, font, color_, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
#endif
        break;

#ifdef USE_Fireplugin
      case MODE_FIRE:
      {
        int16_t x = 0;
        int16_t y = 0;

        for (x = 0; x < 32; ++x)
        {
          /* Step 1) Cool down every cell a little bit */
          for (y = 0; y < 8; ++y)
          {
            uint8_t coolDownTemperature = random(0, ((COOLING * 10U) / 8)) + 2U;
            uint32_t heatPos = x + y * 32;

            if (coolDownTemperature >= m_heat[heatPos])
            {
              m_heat[heatPos] = 0U;
            }
            else
            {
              m_heat[heatPos] -= coolDownTemperature;
            }
          }

          /* Step 2) Heat from each cell drifts 'up' and diffuses a little bit */
          for (y = 0; y < (8 - 1U); ++y)
          {
            uint16_t diffusHeat = 0U;

            if ((8 - 2U) > y)
            {
              diffusHeat += m_heat[x + (y + 1) * 32];
              diffusHeat += m_heat[x + (y + 1) * 32];
              diffusHeat += m_heat[x + (y + 2) * 32];
              diffusHeat /= 3U;
            }
            else
            {
              diffusHeat += m_heat[x + (y + 0) * 32];
              diffusHeat += m_heat[x + (y + 0) * 32];
              diffusHeat += m_heat[x + (y + 1) * 32];
              diffusHeat /= 3U;
            }

            m_heat[x + y * 32] = diffusHeat;
          }

          /* Step 3) Randomly ignite new 'sparks' of heat near the bottom */
          if (random(0, 255) < SPARKING)
          {
            uint8_t randValue = random(160, 255);
            uint32_t heatPos = x + (8 - 1U) * 32;
            uint16_t heat = m_heat[heatPos] + randValue;

            if (UINT8_MAX < heat)
            {
              m_heat[heatPos] = 255U;
            }
            else
            {
              m_heat[heatPos] = heat;
            }
          }

          /* Step 4) Map from heat cells to LED colors */
          for (y = 0; y < 8; ++y)
          {
            this->config_->display->draw_pixel_at(x, y, heatColor(m_heat[x + y * 32]));
          }
        }
      }
      break;
#endif

      default:
        ESP_LOGD(TAG, "no screen to draw!");
        this->config_->next_action_time = 0;
        break;
      }
      this->update_screen();
    }
  }

  void EHMTX_queue::hold_slot(uint8_t _sec)
  {
    this->endtime += _sec * 1000;
    ESP_LOGD(TAG, "hold for %d secs", _sec);
  }

  // TODO void EHMTX_queue::set_mode_icon()

  void EHMTX_queue::calc_scroll_time(std::string text, uint16_t screen_time)
  {
    int x, y, w, h;
    float display_duration;

    uint8_t width = 32;
    uint8_t startx = 0;
    uint16_t max_steps = 0;

    if (this->default_font)
    {
      this->config_->display->get_text_bounds(0, 0, text.c_str(), this->config_->default_font, display::TextAlign::LEFT, &x, &y, &w, &h);
    }
    else
    {
      this->config_->display->get_text_bounds(0, 0, text.c_str(), this->config_->special_font, display::TextAlign::LEFT, &x, &y, &w, &h);
    }

    this->pixels_ = w;

    switch (this->mode)
    {
    case MODE_TEXT_SCREEN:
    case MODE_RAINBOW_TEXT:
#ifdef EHMTXv2_SCROLL_SMALL_TEXT
      max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
      display_duration = max_steps * EHMTXv2_SCROLL_INTERVALL;
      this->screen_time_ = (display_duration > screen_time) ? display_duration : screen_time;
#else
      if (this->pixels_ < 32)
      {
        this->screen_time_ = screen_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = max_steps * EHMTXv2_SCROLL_INTERVALL;
        this->screen_time_ = (display_duration > screen_time) ? display_duration : screen_time;
      }
#endif
      break;
    case MODE_RAINBOW_ICON:
    case MODE_BITMAP_SMALL:
    case MODE_RAINBOW_BITMAP_SMALL:
    case MODE_ICON_SCREEN:
    case MODE_ALERT_SCREEN:
    case MODE_ICON_PROGRESS:
      startx = 8;
      if (this->pixels_ < 23)
      {
        this->screen_time_ = screen_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = max_steps * EHMTXv2_SCROLL_INTERVALL;
        this->screen_time_ = (display_duration > screen_time) ? display_duration : screen_time;
      }
      break;
    case MODE_ICON_TEXT_SCREEN:
    case MODE_RAINBOW_ICON_TEXT_SCREEN:
      if (this->pixels_ < 23)
      {
        this->screen_time_ = screen_time;
      }
      else
      {
        max_steps = EHMTXv2_SCROLL_COUNT * (width - startx) + EHMTXv2_SCROLL_COUNT * this->pixels_;
        display_duration = max_steps * EHMTXv2_SCROLL_INTERVALL;
        this->screen_time_ = (display_duration > screen_time) ? display_duration : screen_time;
      }
      break;
    default:
      break;
    }

    this->scroll_reset = (width - startx) + this->pixels_;
    ;

    ESP_LOGD(TAG, "calc_scroll_time: mode: %d text: \"%s\" pixels %d calculated: %d defined: %d max_steps: %d", this->mode, text.c_str(), this->pixels_, this->screen_time_, screen_time, this->scroll_reset);
  }
  
}
