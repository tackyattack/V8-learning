//
//  main.cpp
//  V8_learning
//
//  Created by HENRY BERGIN on 3/2/18.
//  Copyright © 2018 HENRY BERGIN. All rights reserved.
//
// Note: Make sure the snapshot_blob and natives_blob reside with the executable! V8 needs to be there to
//       be able to run.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <iostream>
#include "libplatform/libplatform.h"
#include "v8.h"

static int x = 100;

static void XGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    info.GetReturnValue().Set(x);
}

// NOTE: The wiki has the PropertyCallbackInfo<xxx> wrong
//       it should be "void", not "Value"
static void XSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    x = value->Int32Value();
}

void printCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    // create a stack-allocated handle scope for the arguments
    // note: this may not be necessary since V8 probably
    //       takes care of putting the arguments on the stack.
    //       So, if you were to return a local handle from
    //       a function, it would need to use Escape
    v8::HandleScope handle_scope(args.GetIsolate());
    
    // step through each argument
    for (int i = 0; i < args.Length(); i++)
    {
        // create a string object from the argument
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        char *print_str = *str; // cast the string to char type
        printf("%s\n",print_str);
    }
}




int main(int argc, char* argv[]) {
    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    v8::Platform* platform = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();
    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
    v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    
    
    
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        
        // Create a stack-allocated handle scope.
        // This holds a bunch of handles (used to point to JS objects)
        // Makes deallocation easier
        v8::HandleScope handle_scope(isolate);
        
        // Create a template for the global object
        // I.e. objects that can be accessed anywhere in the script
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
        
        // Setup the print function in the global space
        global->Set(
                    v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal)
                    .ToLocalChecked(),
                    v8::FunctionTemplate::New(isolate, printCallback));
        
        //
        global->SetAccessor(v8::String::NewFromUtf8(isolate, "x", v8::NewStringType::kNormal).ToLocalChecked(), XGetter, XSetter);

        
        
        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
        
        // Enter the context for compiling and running the hello world script.
        // A context is the environment that allows the JavaScript application
        // to run in.
        v8::Context::Scope context_scope(context);
        
        // Create a string containing the JavaScript source code.
        char scriptst[] = "print(\"Hello World!\"); x";
        v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, scriptst,
                                v8::NewStringType::kNormal).ToLocalChecked();
        
        // Compile the source code.
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        
        // Run the script to get the result.
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        
        // Convert the result to an UTF8 string and print it.
        v8::String::Utf8Value utf8(result);
        printf("%s\n", *utf8);
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete platform;
    delete create_params.array_buffer_allocator;
    return 0;
}
