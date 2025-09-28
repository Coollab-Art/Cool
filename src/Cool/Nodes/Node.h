#pragma once
#include "Pin.h"

namespace Cool {

template<typename T>
concept Node_Concept = requires(T mut_node, T const const_node) { // clang-format off
    // { node.name() } -> std::convertible_to<std::string>;
    
    { mut_node.input_pins() } -> std::convertible_to<std::vector<Cool::InputPin>&>;
    { const_node.input_pins() } -> std::convertible_to<std::vector<Cool::InputPin> const&>;
    { mut_node.output_pins() } -> std::convertible_to<std::vector<Cool::OutputPin>&>;
    { const_node.output_pins() } -> std::convertible_to<std::vector<Cool::OutputPin> const&>;
    { const_node.definition_name() } -> std::convertible_to<std::string>;
    { const_node.category_name() } -> std::convertible_to<std::string>;
    { mut_node.name_ref() } -> std::convertible_to<std::string&>;
}; // clang-format on

class Node {
public:
    [[nodiscard]] auto input_pins() -> std::vector<Cool::InputPin>& { return _pimpl->get_input_pins(); }
    [[nodiscard]] auto input_pins() const -> std::vector<Cool::InputPin> const& { return _pimpl->get_input_pins(); }
    [[nodiscard]] auto output_pins() -> std::vector<Cool::OutputPin>& { return _pimpl->get_output_pins(); }
    [[nodiscard]] auto output_pins() const -> std::vector<Cool::OutputPin> const& { return _pimpl->get_output_pins(); }
    [[nodiscard]] auto definition_name() const -> std::string { return _pimpl->get_definition_name(); }
    [[nodiscard]] auto category_name() const -> std::string { return _pimpl->get_category_name(); }
    [[nodiscard]] auto name_ref() -> std::string& { return _pimpl->name_ref(); }

    template<Node_Concept NodeT>
    [[nodiscard]] auto downcast() -> NodeT&
    {
        return static_cast<Model<NodeT>&>(*_pimpl)._node;
    }
    template<Node_Concept NodeT>
    [[nodiscard]] auto downcast() const -> NodeT const&
    {
        return static_cast<Model<NodeT> const&>(*_pimpl)._node;
    }

public: // Type-erasure implementation details
    Node() = default;

    template<Node_Concept NodeT>
    Node(NodeT node) // NOLINT(*-explicit-constructor, *-explicit-conversions) A type-erased object should be implicitly created from objects matching its requirements.
        : _pimpl{std::make_unique<Model<NodeT>>(std::move(node))}
    {}

    Node(Node const& other)
        : _pimpl{other._pimpl->clone()}
    {}
    auto operator=(Node const& other) -> Node&
    {
        auto tmp = Node{other};
        std::swap(_pimpl, tmp._pimpl);
        return *this;
    }
    Node(Node&&) noexcept                    = default;
    auto operator=(Node&&) noexcept -> Node& = default;
    ~Node()                                  = default;

public:              // Must be public in order for ser20 to register the polymorphic types
    struct Concept { // NOLINT(*-special-member-functions)
        virtual ~Concept() = default;

        [[nodiscard]] virtual auto get_input_pins() -> std::vector<Cool::InputPin>&               = 0;
        [[nodiscard]] virtual auto get_input_pins() const -> std::vector<Cool::InputPin> const&   = 0;
        [[nodiscard]] virtual auto get_output_pins() -> std::vector<Cool::OutputPin>&             = 0;
        [[nodiscard]] virtual auto get_output_pins() const -> std::vector<Cool::OutputPin> const& = 0;
        [[nodiscard]] virtual auto get_definition_name() const -> std::string                     = 0;
        [[nodiscard]] virtual auto get_category_name() const -> std::string                       = 0;
        [[nodiscard]] virtual auto name_ref() -> std::string&                                     = 0;

        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Concept> = 0;
    };

    template<Node_Concept NodeT>
    struct Model : public Concept {
        Model() = default;
        explicit Model(NodeT def)
            : _node{std::move(def)}
        {}

        [[nodiscard]] auto get_input_pins() -> std::vector<Cool::InputPin>& override
        {
            return _node.input_pins();
        }
        [[nodiscard]] auto get_input_pins() const -> std::vector<Cool::InputPin> const& override
        {
            return _node.input_pins();
        }
        [[nodiscard]] auto get_output_pins() -> std::vector<Cool::OutputPin>& override
        {
            return _node.output_pins();
        }
        [[nodiscard]] auto get_output_pins() const -> std::vector<Cool::OutputPin> const& override
        {
            return _node.output_pins();
        }
        [[nodiscard]] auto get_definition_name() const -> std::string override
        {
            return _node.definition_name();
        }
        [[nodiscard]] auto get_category_name() const -> std::string override
        {
            return _node.category_name();
        }
        [[nodiscard]] auto name_ref() -> std::string& override
        {
            return _node.name_ref();
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Concept> override
        {
            return std::make_unique<Model>(*this);
        }

        NodeT _node;

    private:
        // Serialization
        friend class ser20::access;
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                ser20::make_nvp("Node", _node)
            );
        }
    };

private:
    std::unique_ptr<Concept> _pimpl;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("pimpl", _pimpl)
        );
    }
};

} // namespace Cool