// ========================================================================== //
// This file is part of DO++, a basic set of libraries in C++ for computer 
// vision.
//
// Copyright (C) 2014 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public 
// License v. 2.0. If a copy of the MPL was not distributed with this file, 
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

// Google Test.
#include <gtest/gtest.h>
// Qt libraries.
#include <QtTest>
// DO-CV libraries.
#include <DO/Graphics/DerivedQObjects/OpenGLWindow.hpp>
// Local libraries.
#include "event_scheduler.hpp"

Q_DECLARE_METATYPE(DO::Event);
Q_DECLARE_METATYPE(Qt::Key);
Q_DECLARE_METATYPE(Qt::MouseButtons);

using namespace DO;

TEST(TestOpenGLWindow, test_construction)
{
  int width = 300;
  int height = 300;
  QString windowName = "OpenGL Window";
  int x = 200;
  int y = 300;

  OpenGLWindow *window = new OpenGLWindow(
    width, height,
    windowName,
    x, y
  );

  EXPECT_EQ(window->width(), width);
  EXPECT_EQ(window->height(), height);
  EXPECT_EQ(window->windowTitle(), windowName);
  EXPECT_TRUE(window->isVisible());

  delete window;
}


class TestOpenGLWindowEvents: public testing::Test
{
protected: // data members.
  OpenGLWindow *test_window_;
  EventScheduler event_scheduler_;
  QPoint mouse_pos_;
  Qt::Key key_;
  int mouse_buttons_type_id_;
  int event_type_id_;

protected: // methods.
  TestOpenGLWindowEvents()
  {
    mouse_buttons_type_id_ = qRegisterMetaType<Qt::MouseButtons>(
      "Qt::MouseButtons"
      );
    event_type_id_ = qRegisterMetaType<Event>("Event");
    test_window_ = new OpenGLWindow(300, 300);
    event_scheduler_.set_receiver(test_window_);
    mouse_pos_ = QPoint(10, 10);
    key_ = Qt::Key_A;
  }

  virtual ~TestOpenGLWindowEvents()
  {
    delete test_window_;
  }

  void compare_key_event(QSignalSpy& spy) const
  {
    spy.wait(500);
    EXPECT_EQ(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), static_cast<int>(key_));
  }

};

TEST_F(TestOpenGLWindowEvents, test_key_press_event)
{
  QSignalSpy spy(test_window_, SIGNAL(pressedKey(int)));
  EXPECT_TRUE(spy.isValid());

  QKeyEvent event(QEvent::KeyPress, key_, Qt::NoModifier);
  event_scheduler_.schedule_event(&event, 10);

  compare_key_event(spy);
}

TEST_F(TestOpenGLWindowEvents, test_key_release_event)
{
  QSignalSpy spy(test_window_, SIGNAL(releasedKey(int)));
  EXPECT_TRUE(spy.isValid());

  QKeyEvent event(QEvent::KeyRelease, key_, Qt::NoModifier);
  event_scheduler_.schedule_event(&event, 10);

  compare_key_event(spy);
}

TEST_F(TestOpenGLWindowEvents, test_send_no_event)
{
  QSignalSpy spy(test_window_, SIGNAL(sendEvent(Event)));
  EXPECT_TRUE(spy.isValid());

  QMetaObject::invokeMethod(test_window_, "waitForEvent",
    Qt::AutoConnection, Q_ARG(int, 1));

  // Nothing happens.
  EXPECT_TRUE(spy.wait(10));
  EXPECT_EQ(spy.count(), 1);
  QList<QVariant> arguments = spy.takeFirst();
  QVariant arg = arguments.at(0);
  arg.convert(event_type_id_);
  Event event(arguments.at(0).value<Event>());
  EXPECT_EQ(event.type, DO::NO_EVENT);
}

TEST_F(TestOpenGLWindowEvents, test_send_pressed_key_event)
{
  // Spy the sendEvent signal.
  QSignalSpy spy(test_window_, SIGNAL(sendEvent(Event)));
  EXPECT_TRUE(spy.isValid());

  // Ask the testing window to wait for 10 ms.
  QMetaObject::invokeMethod(test_window_, "waitForEvent",
                            Qt::AutoConnection, Q_ARG(int, 10));

  // Schedule a key press event 1 ms later.
  QKeyEvent qt_event(QEvent::KeyPress, key_, Qt::NoModifier);
  event_scheduler_.schedule_event(&qt_event, 1);

  // The spy waits for the event during 100 ms.
  EXPECT_TRUE(spy.wait(100));

  // Check that the spy received one key press event.
  EXPECT_EQ(spy.count(), 1);
  
  // Check the details of the key press event.
  QList<QVariant> arguments = spy.takeFirst();
  QVariant arg = arguments.at(0);
  arg.convert(event_type_id_);
  Event event(arguments.at(0).value<Event>());
  EXPECT_EQ(event.type, DO::KEY_PRESSED);
  EXPECT_EQ(event.key, key_);
}

TEST_F(TestOpenGLWindowEvents, test_send_released_key_event)
{
  // Spy the sendEvent signal.
  QSignalSpy spy(test_window_, SIGNAL(sendEvent(Event)));
  EXPECT_TRUE(spy.isValid());

  // Ask the testing window to wait for 10 ms.
  QMetaObject::invokeMethod(test_window_, "waitForEvent",
                            Qt::AutoConnection, Q_ARG(int, 10));

  // Schedule a key press event 1 ms later.
  QKeyEvent qt_event(QEvent::KeyRelease, key_, Qt::NoModifier);
  event_scheduler_.schedule_event(&qt_event, 1);

  // Check that the spy received one key press event.
  EXPECT_TRUE(spy.wait(100));
  EXPECT_EQ(spy.count(), 1);

  // Check the details of the key release event.
  QList<QVariant> arguments = spy.takeFirst();
  QVariant arg = arguments.at(0);
  arg.convert(event_type_id_);
  Event event(arguments.at(0).value<Event>());
  EXPECT_EQ(event.type, DO::KEY_RELEASED);
  EXPECT_EQ(event.key, key_);
}


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setAttribute(Qt::AA_Use96Dpi, true);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}