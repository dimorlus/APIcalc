// Структура для хранения параметров графика
struct PlotParams
{
 char *sexpr;          // Выражение функции
 char *svar;           // Переменная
 float__t vfrom;       // Начало диапазона
 float__t vto;         // Конец диапазона
 float__t ymin;        // Минимум Y
 float__t ymax;        // Максимум Y
 int width;            // Ширина изображения
 int height;           // Высота изображения
 int padding;          // Отступы
 uint32_t bgc;         // Цвет фона
 uint32_t fgc;         // Цвет линии графика
 uint32_t grid_color;  // Цвет сетки
 uint32_t axis_color;  // Цвет осей
 uint32_t text_color;  // Цвет текста
 calculator *child;    // Калькулятор для вычислений
};

// Подготовка данных для построения графика (общая часть)
bool calculator::PlotPrepare (const char *expr, v_func fidx, char *fname, PlotParams &params)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return false;
  }

 char sexpr[STRBUF];
 char sfrom[MAXOP];
 char sto[MAXOP];
 char svar[STRBUF];

 bool split_ok = false;
 if ((fidx == pl_fplot) || (fidx == pl_oplot))
  split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                    nullptr, 0);
 else
  split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);

 if (!split_ok)
  {
   result_fval = qnan;
   return false;
  }

 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }

 if ((fidx == pl_fplot) || (fidx == pl_oplot))
  {
   child->setFileDlgFn (FileDlgFn);
   child->evaluate_f (fname);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }

   if (child->get_res_tag () != tvSTR)
    {
     errorf (pos, "First argument is not a file name");
     delete child;
     result_fval = qnan;
     return false;
    }
   strncpy (fname, child->get_str_res (), STRBUF - 1);
   fname[STRBUF - 1] = '\0';
  }

 float__t vfrom = child->evaluate_f (sfrom);
 if (isnan (vfrom) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 float__t vto = child->evaluate_f (sto);
 if (isnan (vto) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 if (vfrom > vto)
  {
   float__t tmp = vfrom;
   vfrom        = vto;
   vto          = tmp;
  }

 child->addfvar (svar, vfrom);
 float__t fvx = child->evaluate_f (sexpr);

 if (isnan (fvx) || child->err[0] || !CheckChildRes (child))
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 // Заполняем параметры
 params.sexpr = strdup (sexpr);
 params.svar  = strdup (svar);
 params.vfrom = vfrom;
 params.vto   = vto;
 params.ymin  = fvx;
 params.ymax  = fvx;

 params.width = getivar ("plot_width");
 if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

 params.height = getivar ("plot_height");
 if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

 params.bgc     = getivar ("plot_bgc");
 params.fgc     = getivar ("plot_fgc");
 params.padding = 40;

 params.grid_color = 0xC0C0C0;
 params.axis_color = 0x808080;
 params.text_color = ~params.bgc;

 params.child = child;

 return true;
}

// Построение декартового графика (специфичная часть)
bool calculator::PlotCartesian (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;
 float__t ymin     = params.ymin;
 float__t ymax     = params.ymax;

 int width         = params.width;
 int height        = params.height;
 int padding       = params.padding;
 uint32_t fgc      = params.fgc;

 float__t step           = (vto - vfrom) / ((width - 2 * padding) / 4);
 float__t save_vfrom     = vfrom;
 bool has_valid_points   = false;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find ymin/ymax
 for (int pass = 0; pass < 2; pass++)
  {
   do
    {
     if (check_break (init_ms, last_gui_check) != brNONE)
      {
       return false;
      }

     child->addfvar (params.svar, vfrom);
     float__t fvx = child->evaluate_f (params.sexpr);

     if (pass == 0)
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         if (fvx < ymin) ymin = fvx;
         if (fvx > ymax) ymax = fvx;
        }
      }
     else
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         float__t x = padding + ((vfrom - save_vfrom) / (vto - save_vfrom)) * (width - 2 * padding);
         float__t y = height - padding - ((fvx - ymin) / (ymax - ymin)) * (height - 2 * padding);
         if (has_valid_points)
          {
           bmp->lineTo ((int)x, (int)y, 2, fgc);
          }
         else
          {
           bmp->moveTo ((int)x, (int)y);
           has_valid_points = true;
          }
        }
       else
        has_valid_points = false;
      }
     vfrom += step;
    }
   while (vfrom <= vto);
   vfrom = save_vfrom;

   if (pass == 0)
    {
     // Включаем ноль в диапазон
     if (ymin > 0.0) ymin = 0.0;
     if (ymax < 0.0) ymax = 0.0;

     // Добавляем padding
     if (ymin == ymax)
      {
       ymin -= (float__t)1.0L;
       ymax += (float__t)1.0L;
      }
     else
      {
       float__t ypad = (ymax - ymin) * (float__t)0.1L;
       ymin -= ypad;
       ymax += ypad;
      }
    }
  }

 // Обновляем параметры для отрисовки осей и сетки
 params.ymin = ymin;
 params.ymax = ymax;

 return true;
}

// Отрисовка осей и сетки (декартовы координаты)
void calculator::PlotDrawAxesCartesian (bmpdraw *bmp, PlotParams &params)
{
 int width       = params.width;
 int height      = params.height;
 int padding     = params.padding;
 float__t ymin   = params.ymin;
 float__t ymax   = params.ymax;
 float__t vfrom  = params.vfrom;
 float__t vto    = params.vto;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 float__t x_range = vto - vfrom;
 float__t y_range = ymax - ymin;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int y_axis_pixel = height - padding - ((0.0 - ymin) / y_range) * plot_height;

 int x_axis_pixel = -1;
 if (vfrom <= 0.0 && vto >= 0.0)
  {
   x_axis_pixel = padding + ((0.0 - vfrom) / x_range) * plot_width;
  }

 // Горизонтальные линии сетки
 for (int offset = 0; offset <= plot_height; offset += grid_step_pixels)
  {
   int y_up   = y_axis_pixel - offset;
   int y_down = y_axis_pixel + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4)
      bmp->drawPixel (x, y_up, grid_color);
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4)
      bmp->drawPixel (x, y_down, grid_color);
    }
  }

 // Вертикальные линии сетки
 if (x_axis_pixel >= 0)
  {
   for (int offset = 0; offset <= plot_width; offset += grid_step_pixels)
    {
     int x_left  = x_axis_pixel - offset;
     int x_right = x_axis_pixel + offset;

     if (x_left >= padding && x_left < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4)
        bmp->drawPixel (x_left, y, grid_color);
      }

     if (offset > 0 && x_right >= padding && x_right < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4)
        bmp->drawPixel (x_right, y, grid_color);
      }
    }
  }
 else
  {
   for (int x = padding; x < width - padding; x += grid_step_pixels)
    {
     for (int y = padding; y < height - padding; y += 4)
      bmp->drawPixel (x, y, grid_color);
    }
  }

 // Оси
 if (x_axis_pixel >= 0)
  {
   bmp->drawLine (x_axis_pixel, padding, x_axis_pixel, height - padding, 1, axis_color);
  }
 bmp->drawLine (padding, y_axis_pixel, width - padding, y_axis_pixel, 1, axis_color);

 // Подписи осей
 char label[64];

 d2scistr (label, (double)vfrom);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)vto);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 char title[128];
 snprintf (title, sizeof (title), "y=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);
}

// Главная функция построения графика
bool calculator::Plot (const char *expr, v_func fidx)
{
 char fname[STRBUF];
 fname[0] = '\0';

 PlotParams params;
 memset (&params, 0, sizeof (params));

 // 1. Подготовка данных
 if (!PlotPrepare (expr, fidx, fname, params))
  {
   return false;
  }

 // 2. Создание bitmap
 bmpdraw *bmp = new bmpdraw ();
 if (!bmp || !bmp->newbmp (params.width, params.height, params.bgc))
  {
   errorf (pos, "Failed to create bitmap for plotting");
   delete bmp;
   if (params.child) delete params.child;
   if (params.sexpr) free (params.sexpr);
   if (params.svar) free (params.svar);
   result_fval = qnan;
   return false;
  }

 // 3. Построение графика (декартовы координаты)
 if (!PlotCartesian (bmp, params))
  {
   delete bmp;
   delete params.child;
   free (params.sexpr);
   free (params.svar);
   result_fval = qnan;
   return false;
  }

 // 4. Отрисовка осей и сетки
 PlotDrawAxesCartesian (bmp, params);

 // 5. Сохранение или показ
 if ((fidx == pl_fplot) || (fidx == pl_oplot))
  {
   bmp->save (fname);
  }
 else
  {
   if (ShowImageFn)
    {
     ShowImageFn ((void *)bmp);
    }
  }

 // 6. Очистка
 delete bmp;
 fflags |= params.child->isfflags ();
 delete params.child;
 free (params.sexpr);
 free (params.svar);

 return true;
}