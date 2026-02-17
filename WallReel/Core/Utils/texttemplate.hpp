#ifndef TEXTTEMPLATE_HPP
#define TEXTTEMPLATE_HPP

#include <QMap>
#include <QRegularExpression>
#include <QString>

namespace WallReel::Core::Utils {

/**
 * @brief Replaces {{ key }} style placeholders in a template string with corresponding values from a map.
 *
 * Supports:
 *   - Whitespace tolerance: {{ key }}, {{key}}, {{  key  }} are all valid.
 *   - Missing keys are left as-is (no replacement).
 *   - Nested braces or malformed placeholders are ignored.
 *   - Empty keys are ignored.
 *   - Keys are trimmed before lookup.
 *
 * @param templateStr The template string containing {{ key }} placeholders.
 * @param variables   A map of key-value pairs for substitution.
 * @return The rendered string with placeholders replaced.
 */
inline QString renderTemplate(const QString& templateStr, const QMap<QString, QString>& variables) {
    if (templateStr.isEmpty() || variables.isEmpty()) {
        return templateStr;
    }

    // Match {{ ... }} with possible whitespace around the key.
    // Use a non-greedy match for the content inside braces to handle multiple placeholders correctly.
    static const QRegularExpression regex(
        QStringLiteral(R"(\{\{\s*([^{}]+?)\s*\}\})"),
        QRegularExpression::DontCaptureOption);

    // We need the capture group, so rebuild without DontCaptureOption
    static const QRegularExpression placeholderRegex(
        QStringLiteral(R"(\{\{\s*([^{}]+?)\s*\}\})"));

    QString result;
    result.reserve(templateStr.size());

    qsizetype lastPos                  = 0;
    QRegularExpressionMatchIterator it = placeholderRegex.globalMatch(templateStr);

    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const qsizetype matchStart          = match.capturedStart(0);
        const qsizetype matchLength         = match.capturedLength(0);
        const QString key                   = match.captured(1).trimmed();

        // Append everything before this match
        result.append(templateStr.mid(lastPos, matchStart - lastPos));

        if (!key.isEmpty() && variables.contains(key)) {
            // Replace with the value from the map
            result.append(variables.value(key));
        } else {
            // Key not found or empty — leave the placeholder as-is
            result.append(match.captured(0));
        }

        lastPos = matchStart + matchLength;
    }

    // Append any remaining text after the last match
    if (lastPos < templateStr.size()) {
        result.append(templateStr.mid(lastPos));
    }

    return result;
}

/**
 * @brief Overload accepting QHash for convenience.
 */
inline QString renderTemplate(const QString& templateStr, const QHash<QString, QString>& variables) {
    if (templateStr.isEmpty() || variables.isEmpty()) {
        return templateStr;
    }

    static const QRegularExpression placeholderRegex(
        QStringLiteral(R"(\{\{\s*([^{}]+?)\s*\}\})"));

    QString result;
    result.reserve(templateStr.size());

    qsizetype lastPos                  = 0;
    QRegularExpressionMatchIterator it = placeholderRegex.globalMatch(templateStr);

    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const qsizetype matchStart          = match.capturedStart(0);
        const qsizetype matchLength         = match.capturedLength(0);
        const QString key                   = match.captured(1).trimmed();

        result.append(templateStr.mid(lastPos, matchStart - lastPos));

        if (!key.isEmpty() && variables.contains(key)) {
            result.append(variables.value(key));
        } else {
            result.append(match.captured(0));
        }

        lastPos = matchStart + matchLength;
    }

    if (lastPos < templateStr.size()) {
        result.append(templateStr.mid(lastPos));
    }

    return result;
}

/**
 * @brief Extracts all placeholder keys from a template string.
 *
 * @param templateStr The template string to scan.
 * @return A list of unique keys found in the template (trimmed).
 */
inline QStringList extractTemplateKeys(const QString& templateStr) {
    static const QRegularExpression placeholderRegex(
        QStringLiteral(R"(\{\{\s*([^{}]+?)\s*\}\})"));

    QSet<QString> seen;
    QStringList keys;

    QRegularExpressionMatchIterator it = placeholderRegex.globalMatch(templateStr);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString key                   = match.captured(1).trimmed();
        if (!key.isEmpty() && !seen.contains(key)) {
            seen.insert(key);
            keys.append(key);
        }
    }

    return keys;
}

}  // namespace WallReel::Core::Utils

#endif  // TEXTTEMPLATE_HPP
