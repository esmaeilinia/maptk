/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name Kitware, Inc. nor the names of any contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <maptk/version.h>

#include <qtSaxNodes.h>
#include <qtSaxWriter.h>
#include <qtUtil.h>

#include <QtGui/QDesktopServices>

#include <QtCore/QFile>
#include <QtCore/QUrl>

QTE_IMPLEMENT_D_FUNC(AboutDialog)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QString formatTitle(QString formatStr)
{
  formatStr.replace("@APP_TITLE@", QApplication::applicationName());
  formatStr.replace("@APP_VERSION@", QApplication::applicationVersion());
  formatStr.replace("@QT_VERSION@", QString::fromLocal8Bit(qVersion()));
  return formatStr;
}

//-----------------------------------------------------------------------------
QString buildCopyrightText()
{
  QString format = "Copyright &copy;%1 %2";
  return format.arg(MAPTK_COPYRIGHT_YEAR).arg(QApplication::organizationName());
}

//-----------------------------------------------------------------------------
QString loadText(QString const& resource)
{
  QFile f(resource);
  f.open(QIODevice::ReadOnly);
  return f.readAll();
}

//-----------------------------------------------------------------------------
QString loadMarkdown(QString const& resource)
{
  auto input = loadText(resource);
  input.replace("&", "&amp;");
  input.replace("<", "&lt;");
  input.replace(">", "&gt;");
  input.replace("``", "&ldquo;");
  input.replace("''", "&rdquo;");
  input.replace("`", "&lsquo;");
  input.replace("'", "&rsquo;");

  QString markup;
  qtSaxWriter out(&markup);
  out << qtSaxElement("html") << qtSaxElement("body");

  auto depth = 2;
  foreach (auto const& block, input.split("\n\n"))
  {
    auto const text = block.simplified();
    if (text.startsWith("*"))
    {
      if (depth < 3)
      {
        out << qtSaxElement("ul")
            << qtSaxAttribute("style", "margin-left: 1.5em;"
                                       "-qt-list-indent: 0;");
        ++depth;
      }
      out << qtSaxElement("li")
          << qtSaxAttribute("style", "margin-bottom: 1em;")
          << qtSaxText(text.mid(1).trimmed(), qtSaxText::TextWithEntities)
          << qtSax::EndElement;
    }
    else
    {
      while (depth > 2)
      {
        out << qtSax::EndElement;
        --depth;
      }
      out << qtSaxElement("p")
          << qtSaxText(text, qtSaxText::TextWithEntities)
          << qtSax::EndElement;
    }
  }
  while (depth)
  {
    out << qtSax::EndElement;
    --depth;
  }

  return markup;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class AboutDialogPrivate
{
public:
  Ui::AboutDialog UI;
};

//-----------------------------------------------------------------------------
AboutDialog::AboutDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), d_ptr(new AboutDialogPrivate)
{
  QTE_D(AboutDialog);

  // Set up UI
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);
  this->setWindowTitle(QString("About %1").arg(qApp->applicationName()));

  connect(d->UI.about, SIGNAL(linkActivated(QString)),
          this, SLOT(openLink(QString)));

  // Fill title and copyright texts
  d->UI.title->setText(formatTitle(d->UI.title->text()));
  d->UI.copyright->setText(buildCopyrightText());

  // Load various supplemental texts
  d->UI.acknowledgments->setText(loadMarkdown(":/ACKNOWLEDGMENTS"));
  d->UI.license->setHtml(loadMarkdown(":/LICENSE"));
  d->UI.buildInfo->setHtml(loadText(":/BUILDINFO"));
}

//-----------------------------------------------------------------------------
AboutDialog::~AboutDialog()
{
}

//-----------------------------------------------------------------------------
void AboutDialog::openLink(QString const& uri)
{
  QDesktopServices::openUrl(QUrl(uri));
}
