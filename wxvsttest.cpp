#include <windows.h>
#include <sstream>
#include "wxvsttest.hpp"
#include <borg/msg.hpp>

VST_PLUGIN(Test)

#if WIN32
# include <windows.h>
extern "C"
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
  MSG_BOX("DllMain");
  
  switch(dwReason)
  {
    case DLL_PROCESS_ATTACH:
    {
      vst::wxeditor::init(hInst);
      break;
    }
    case DLL_PROCESS_DETACH:
    {
      vst::wxeditor::shutdown();
      break;
    }
  }
  return TRUE;
}
#endif // WIN32

//---------------------------------------------------------------------------
TestEditor::TestEditor(vst::plugin &vstplugin, buffer<float> &output) :
  vst::wxeditor(vstplugin, borg::size<int>(400, 300)),
  isOpen(false)
//  size(0, 0, 400, 300),
//  test(0)
{
  vstplugin.setEditor(this);
  
/*  rect.left = 0;
  rect.top = 0;
  rect.right = size.width();
  rect.bottom = size.height();*/
  
//  scope = new scope_drawer<vstgui::drawable>(output, 20000);
}

//---------------------------------------------------------------------------
TestEditor::~TestEditor()
{
//  if(frame) delete frame; frame = 0;
//  if(test) delete test; test = 0;
}

//---------------------------------------------------------------------------
bool TestEditor::getRect(ERect **erect)
{
//  *erect = &rect;
  return true;
}
  
//---------------------------------------------------------------------------
bool TestEditor::open(void *ptr)
{
  MSG_BOX("TestEditor::open()");
//  AEffGUIEditor::open(ptr);
//  frame = new CFrame(size, ptr, this);
//  test = new CScopeView(frame, CRect(10, 10, 390, 250), *scope);
//  frame->addView(test);
//  frame->open();

  
//  DrawContext *dc = frame->createDrawContext();  
//  bmp = new CBitmap(frame, "env.png");  
//  frame->setBackground(bmp);
  
  isOpen = true;
  return true;
}

//---------------------------------------------------------------------------
void TestEditor::close()
{
  isOpen = false;
//  if(frame) delete frame; frame = 0;
}

//---------------------------------------------------------------------------
/*void TestEditor::setParameter(long index, float value)
{
  if(!isOpen) return;
}

//---------------------------------------------------------------------------
void TestEditor::valueChanged(CDrawContext *context, CControl *control)
{
  
}*/

//---------------------------------------------------------------------------
void TestEditor::idle()
{
//  AEffGUIEditor::idle();
}

//---------------------------------------------------------------------------
Test::Test()
{
  setNumInputs(2);
  setNumOutputs(2);
  
  canProcessReplacing();
  isSynth();
  setUniqueID(CCONST('t','e','s','t'));
  
//  shakers = new StkShakers();

/*  shakers->controlChange(1071, 8);
  bank.addParameter(new StkFloatParameter(shakers, __SK_Breath_, "Shake Energy", "mag", 0, 2000, 20));
  bank.addParameter(new StkFloatParameter(shakers, __SK_ModFrequency_, "System Decay", "dcy", 0, 1, .5f));*/
//  bank.addParameter(new VST::FloatParameter("Energy", "mag.", 0, 2000, 20, new StkParameterChange(shakers, 2)));
/*  bank.addParameter(new ExpParameter("Freq", "Hz", 20, 10000, 440));
  bank.addParameter(new ExpParameter("Cutoff", "Hz", 20, 10000, 440));
  bank.addParameter(new FloatParameter("Delay", "ms", 1, 1000, 20));
  bank.addParameter(new FloatParameter("Mix", "%", 0, 100, 50));*/  
//  MessageBox(0, "OK", "OK", 0);

  buffer = new reverse_buffer<float>(1024 * 1024);
  editor = new TestEditor(*this, *buffer);
  setEditor(editor);
  
//  scope = new ScopeDrawer(buffer, 512);
}

//---------------------------------------------------------------------------
Test::~Test()
{
}

//-----------------------------------------------------------------------------------------
VstInt32 Test::canDo(char *text)
{
  if(!strcmp(text, "receiveVstEvents")) return 1;
  if(!strcmp(text, "receiveVstMidiEvent")) return 1;
  if(!strcmp(text, "midiProgramNames")) return 1;

  return -1; // explicitly can't do; 0 => don't know
}

//-----------------------------------------------------------------------------------------
VstInt32 Test::getNumMidiInputChannels()
{ 
  return 1; // we are monophonic
}
  
  
//---------------------------------------------------------------------------
void Test::processReplacing(float **inputs, float **outputs,
                            VstInt32 sampleFrames)
{
  float *in1 = inputs[0];
/*  float *in2 = inputs[1];
  float *out1 = outputs[0];
  float *out2 = outputs[1];*/

//  for(int i = 0; i < sampleFrames; i++)
//  {
//    buffer->put( *out1++ = *out2++ = shakers->tick() );
//  }
  
  for(int i = 0; i < sampleFrames; i++)
    buffer->put(*in1++);  
  
  if(editor->isOpen)
  {
/*    editor->frame->setDirty();
    editor->test->setDirty();*/
  }
  
//  msg("buffered %u bytes...", buffer->length());
//  MessageBox(0, "processReplacing", "processReplacing", 0);
}

//---------------------------------------------------------------------------
VstInt32 Test::processEvents(VstEvents *ev)
{
/*  if(ev->events[0])
    msg("numEvents: %i, events[0]: %p", ev->numEvents, ev->events[0]);*/
  
  VstInt32 numEvents = (ev->numEvents ? ev->numEvents : !!ev->events[0]);
  
//  msg("numEvents: %i, events[0]->type: %i",
//      ev->numEvents, ev->events[0]->type);
  
  for(VstInt32 i = 0; i < numEvents; i++)
  {
    if((ev->events[i]) == 0)
    {
      msg("NULL event");
      break;
    }
    
    if((ev->events[i])->type != kVstMidiType)
      continue;

    VstMidiEvent *event = (VstMidiEvent *)ev->events[i];
    char *midiData = event->midiData;
    VstInt32 status = midiData[0] & 0xf0; // ignoring channel

    if(status == 0x90 || status == 0x80) // we only look at notes
    {
      VstInt32 note = midiData[1] & 0x7f;
      VstInt32 velocity = midiData[2] & 0x7f;

      if(status == 0x80)
        velocity = 0; // note off by velocity 0

      if(!velocity && (note == currentNote))
        noteOff();
      else
        noteOn(note, velocity, event->deltaFrames);
    }
    else if(status == 0xb0)
    {
      if(midiData[1] == 0x7e || midiData[1] == 0x7b) // all notes off
        noteOff();
    }
    
    event++;
  }
  return 1;
}

//-----------------------------------------------------------------------------------------
void Test::noteOn(VstInt32 note, VstInt32 velocity, VstInt32 delta)
{
  
//#define msg(x...) do { FILE *fp = stderr; fprintf(fp, x); fputc('\n',fp); fflush(fp); } while(0);
  
//  msg("noteOn: %i, velocity: %i, delta: %i", note, velocity, delta);
  
  
  
  currentNote = note;

//  shakers->noteOn(note, 1);
}

//-----------------------------------------------------------------------------------------
void Test::noteOff()
{
//  shakers->noteOff(1);
}
