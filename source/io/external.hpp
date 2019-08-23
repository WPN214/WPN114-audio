#pragma once
#include <wpn114audio/graph.hpp>

//---------------------------------------------------------------------------------------------
class ExternalBase
//---------------------------------------------------------------------------------------------
{

public:

    ExternalBase() {}
    //---------------------------------------------------------------------------------------------
    virtual
    ~ExternalBase() {}

    //---------------------------------------------------------------------------------------------
    virtual void
    run() = 0;

    //---------------------------------------------------------------------------------------------
    virtual void
    stop() = 0;

    //---------------------------------------------------------------------------------------------
    virtual void
    on_audio_inputs_changed(uint8_t n_inputs) { Q_UNUSED(n_inputs) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_audio_outputs_changed(uint8_t n_outputs) { Q_UNUSED(n_outputs) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_midi_inputs_changed(uint8_t n_inputs) { Q_UNUSED(n_inputs) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_midi_outputs_changed(uint8_t n_outputs) { Q_UNUSED(n_outputs) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_name_changed(QString& name) { Q_UNUSED(name) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_input_midi_targets_changed(QStringList& targets, Routing routing)
    { Q_UNUSED(targets) Q_UNUSED(routing) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_output_midi_targets_changed(QStringList& targets, Routing routing)
    { Q_UNUSED(targets) Q_UNUSED(routing) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_input_audio_targets_changed(QStringList& targets, Routing routing)
    { Q_UNUSED(targets) Q_UNUSED(routing) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_output_audio_targets_changed(QStringList& targets, Routing routing)
    { Q_UNUSED(targets) Q_UNUSED(routing) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_sample_rate_changed(sample_t rate) { Q_UNUSED(rate) }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_io_vector_changed(uint16_t nframes) { Q_UNUSED(nframes) }
};


class ExternalIO;
class Input;
class Output;

//=================================================================================================
class External : public QObject, public QQmlParserStatus
// this is a generic Node wrapper for the external backends
// it embeds an AbstrackBackend object
// and will send it commands and notifications whenever a property changes
//=================================================================================================
{
    Q_OBJECT

public:


    //---------------------------------------------------------------------------------------------
    enum Backend
    //---------------------------------------------------------------------------------------------
    {
        None,

        Default

#ifdef __linux__
        ,Alsa
#endif
#ifdef WPN114AUDIO_JACK
        ,Jack
#endif
#ifdef WPN114AUDIO_PULSEAUDIO
//      ,PulseAudio
#endif
#ifdef __APPLE__
//      ,CoreAudio
#endif
#ifdef WPN114AUDIO_VSTHOST
//      ,VSTHost
#endif
#ifdef ANDROID
        ,Qt
#endif
    };

    Q_ENUM (Backend)

    //---------------------------------------------------------------------------------------------
    Q_INTERFACES (QQmlParserStatus)

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (Backend backend READ backend WRITE set_backend)

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (QString name READ name WRITE set_name)

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (bool running READ running WRITE set_running)

    //---------------------------------------------------------------------------------------------
    ExternalBase*
    m_backend = nullptr;

    //---------------------------------------------------------------------------------------------
    bool
    m_complete  = false,
    m_running   = false;

    //---------------------------------------------------------------------------------------------
    Backend
    m_backend_id = None;

    //---------------------------------------------------------------------------------------------
    QString
    m_name = "wpn114audio";

public:

    //-------------------------------------------------------------------------------------------------
    External() {}

    //-------------------------------------------------------------------------------------------------
    virtual
    ~External() override { delete m_backend; }

    //-------------------------------------------------------------------------------------------------
    virtual void
    classBegin() override {}

    //-------------------------------------------------------------------------------------------------
    virtual void
    componentComplete() override;

    //-------------------------------------------------------------------------------------------------
    Q_SIGNAL void
    connected();

    //-------------------------------------------------------------------------------------------------
    Q_INVOKABLE void
    run()
    //-------------------------------------------------------------------------------------------------
    {
        if (m_complete)
            m_backend->run();
    }

    //-------------------------------------------------------------------------------------------------
    Q_INVOKABLE void
    stop()
    //-------------------------------------------------------------------------------------------------
    {
        if (m_running & m_complete)
            m_backend->stop();
    }

    //-------------------------------------------------------------------------------------------------
    void
    on_rate_changed(sample_t rate) { m_backend->on_sample_rate_changed(rate); }

    //-------------------------------------------------------------------------------------------------
    Backend
    backend() const { return m_backend_id; }

    //-------------------------------------------------------------------------------------------------
    void
    set_backend(Backend backend)
    //-------------------------------------------------------------------------------------------------
    {       
        m_backend_id = backend;
    }

    //---------------------------------------------------------------------------------------------
    bool
    running() const { return m_running; }

    //---------------------------------------------------------------------------------------------
    void
    set_running(bool running)
    //---------------------------------------------------------------------------------------------
    {
        m_running = running;
    }

    //-------------------------------------------------------------------------------------------------
    QString
    name() const { return m_name; }

    //-------------------------------------------------------------------------------------------------
    void
    set_name(QString name) { m_name = name; }

    //---------------------------------------------------------------------------------------------
    void
    register_input(Input* input);

    //---------------------------------------------------------------------------------------------
    void
    register_output(Output* output);

    //---------------------------------------------------------------------------------------------
    std::vector<Input*>&
    midi_inputs() { return m_midi_inputs; }

    //---------------------------------------------------------------------------------------------
    std::vector<Output*>&
    midi_outputs() { return m_midi_outputs; }

    //---------------------------------------------------------------------------------------------
    std::vector<Input*>&
    audio_inputs() { return m_audio_inputs; }

    //---------------------------------------------------------------------------------------------
    std::vector<Output*>&
    audio_outputs() { return m_audio_outputs; }

private:

    std::vector<Input*>
    m_audio_inputs,
    m_midi_inputs;

    std::vector<Output*>
    m_audio_outputs,
    m_midi_outputs;

};

//---------------------------------------------------------------------------------------------
struct ExternalConnection
//---------------------------------------------------------------------------------------------
{
    QString target;
    Routing routing;

    ExternalConnection() {}

    operator
    QVariant() const
    {
        QVariantList output;
        output << target;
        output << routing;

        return output;
    }
};

//-------------------------------------------------------------------------------------------------
class ExternalIO : public Node
//-------------------------------------------------------------------------------------------------
{
    Q_OBJECT

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (Type type READ type WRITE set_type)

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (QVariant channels READ channels WRITE set_channels)

    //---------------------------------------------------------------------------------------------
    Q_PROPERTY  (QVariant connections READ targets WRITE set_targets)

public:

    //---------------------------------------------------------------------------------------------
    enum Type { Audio, Midi }; Q_ENUM (Type)

    //---------------------------------------------------------------------------------------------
    Type
    type() const { return m_type; }

    //---------------------------------------------------------------------------------------------
    void
    set_type(Type t) { m_type = t; }

    //---------------------------------------------------------------------------------------------
    QVariant
    targets() const { return m_targets; }

    //---------------------------------------------------------------------------------------------
    std::vector<ExternalConnection>&
    connections() { return m_connections; }

    //---------------------------------------------------------------------------------------------
    void
    set_targets(QVariant variant) { m_targets = variant; }

    //---------------------------------------------------------------------------------------------
    QVariant
    channels() const { return QVariant::fromValue(m_channels); }

    QVector<int>&
    channels_vec() { return m_channels; }

    nchannels_t
    nchannels() const { return m_channels.size(); }

    //---------------------------------------------------------------------------------------------
    void
    set_channels(QVariant var)
    //---------------------------------------------------------------------------------------------
    {
        if (var.canConvert<int>())
            m_channels.push_back(var.value<int>());

        else if (var.canConvert<QVariantList>())
            for (auto& channel : var.value<QVariantList>())
                set_channels(channel);
    }

    //---------------------------------------------------------------------------------------------
    void
    parse_external_connections()
    //---------------------------------------------------------------------------------------------
    {
        if (m_targets.canConvert<QString>()) {
            ExternalConnection con;
            con.target = m_targets.value<QString>();
            m_connections.push_back(con);
        }

        else if (m_targets.canConvert<QVariantList>())
        {
            auto list = m_targets.value<QVariantList>();
            auto targets = list[0];
            auto routing = list[1];

            if (targets.canConvert<QString>())
            {
                ExternalConnection con;
                con.target = targets.value<QString>();

                if (routing.canConvert<int>()) {
                    // single routing argument
                    uint8_t source = m_channels[0];
                    uint8_t dest = routing.value<uint8_t>();
                    con.routing.append(source, dest);
                }

                else if (routing.canConvert<QVariantList>())
                {
                    auto lrouting = routing.value<QVariantList>();
                    con.routing = Routing(lrouting);

                    for (int n = 0; n < con.routing.ncables(); ++n)
                        // replace source index by channel
                        con.routing[n][0] = m_channels[con.routing[n][0]];
                }

                m_connections.push_back(con);
            }

            else if (targets.canConvert<QVariantList>())
            {
                // TODO...
            }
        }
    }

protected:

    Type
    m_type = Type::Audio;

    QVector<int>
    m_channels;

    std::vector<ExternalConnection>
    m_connections;

    QVariant
    m_targets;
};

//---------------------------------------------------------------------------------------------
class Output : public ExternalIO
//---------------------------------------------------------------------------------------------
{
    Q_OBJECT

    WPN_DECLARE_DEFAULT_AUDIO_INPUT     (audio_in, 0)
    WPN_DECLARE_DEFAULT_MIDI_INPUT      (midi_in, 0)

public:

    //---------------------------------------------------------------------------------------------
    Output() { m_name = "output"; }

    //---------------------------------------------------------------------------------------------
    virtual void
    componentComplete() override
    //---------------------------------------------------------------------------------------------
    {
        parse_external_connections();
        Graph::instance().external()->register_output(this);

        if  (m_type == Type::Audio)
             m_audio_in.set_nchannels(m_channels.count());
        else m_midi_in.set_nchannels(m_channels.count());

        Node::componentComplete();
    }
};

//---------------------------------------------------------------------------------------------
class Input : public ExternalIO
//---------------------------------------------------------------------------------------------
{
    Q_OBJECT

    WPN_DECLARE_DEFAULT_AUDIO_OUTPUT    (audio_out, 0)
    WPN_DECLARE_DEFAULT_MIDI_OUTPUT     (midi_out, 0)

public:

    //---------------------------------------------------------------------------------------------
    Input()
    {
        m_name = "input";
        m_dispatch = Dispatch::Chain;
    }

    //---------------------------------------------------------------------------------------------
    virtual void
    componentComplete() override
    //---------------------------------------------------------------------------------------------
    {
        parse_external_connections();
        Graph::instance().external()->register_input(this);

        if (m_type == Type::Audio)
             m_audio_out.set_nchannels(m_channels.count());
        else m_midi_out.set_nchannels(m_channels.count());

        Node::componentComplete();
    }
};
